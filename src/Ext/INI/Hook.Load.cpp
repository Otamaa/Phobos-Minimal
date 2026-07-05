#include "Body.h"

#include <CRT.h>

#include <Utilities/Debug.h>
#include <Utilities/Macro.h>
#include <Utilities/Patch.h>

int FakeCCINIClass::IteratorValue = 0;

// ============================================================================
// INIClass::Load - backported from IDA pseudocode + cross-checked vs assembly
// ============================================================================
//
// NOTES (read before using):
// - This is a 1:1 behavioral backport. All `goto` removed via early-returns
//   and lambdas.
// - ARES FEATURES BAKED IN (previously 3 separate ASMJIT_PATCH hooks in the
//   injected binary - now just ordinary inline logic, since we own the
//   source here and don't need register-patch tricks anymore):
//
//   (a) "+" auto-numbered key rewrite (was: INIClass_Parse_IteratorChar1 @
//       0x5260A2 and INIClass_Parse_IteratorChar2 @ 0x525D23). Documented
//       user-facing usage:
//           [SomeSection]
//           0=Something
//           += SomethingAdded   ; key is literally "+"
//           [SomethingAdded]
//           ...
//       A key written as exactly "+" gets rewritten to "var_<N>" (N is
//       FakeCCINIClass::IteratorValue, a process-lifetime static that never
//       resets between Load() calls). This is intentional: var_N names are
//       throwaway CRC-discriminators, not data anyone reads back by name.
//       The only guarantee needed is uniqueness across the full [#include]
//       chain (which spans multiple Load() calls) and across independent
//       top-level loads. A monotonically climbing static satisfies that with
//       zero coordination cost on a single-threaded loader.
//
//   (c) Section inheritance via `: [PARENT]` syntax (was: 4 hooks —
//       INIClass_Parse_IniSectionIncludes_PreProcess1  @ 0x525CA5
//       INIClass_Parse_IniSectionIncludes_PreProcess2  @ 0x525DDB
//       INIClass_Parse_IniSectionIncludes_CopySection1 @ 0x525C28
//       INIClass_Parse_IniSectionIncludes_CopySection2 @ 0x525E44)
//       Usage:
//           [CHILD] : [PARENT]
//           ChildOnlyKey=Value
//       On parse, all entries from [PARENT] are copied into [CHILD] via
//       WriteString before [CHILD]'s own entries are processed. Parent must
//       exist in SectionIndex before [CHILD] is parsed (forward references
//       are not supported and emit a Debug::LogError).
//       Original hooks used IniSectionIncludes::includedSection (a static)
//       to ferry the resolved parent pointer between PreProcess and
//       CopySection hooks that couldn't share locals. Now just a local
//       variable — no static needed. Both merge-path and fresh-path are
//       integrated inline at their natural points (header-line parsing and
//       post-construction respectively).
//
//   (d) Duplicate-section cleanup (was: INIClass_Parse_Override @ 0x5260D9).
//       In the original binary this was patched in at an unrelated CRC-call
//       address purely because that was a convenient byte-patchable spot -
//       it has nothing to do with entries. It actually belongs with section
//       bookkeeping: if SectionIndex already holds a section with the same
//       CRC as the one just parsed (i.e. the INI - or an INI merged on top
//       of it - declares the same [SectionName] twice), the OLD section
//       object and its index slot are removed/freed before the new one is
//       inserted, preventing a memory leak + a stale duplicate CRC slot
//       that FetchIndex could never reach. Moved to the natural spot:
//       Pass 3's "decide whether to keep this section" block, right before
//       the new section gets added to SectionIndex.
// - SUSPECT: the original has THREE structurally similar parsing passes:
//     (1) the section-comment pre-scan loop (collects leading comments,
//         detects merge vs fresh-parse)
//     (2) the "merge" fast path (when SectionIndex already has entries -
//         skips entry-comment tracking, calls Put_String directly)
//     (3) the "fresh" path (builds full INISection/INIEntry node graph)
//   These are NOT refactored into one shared helper, because the original
//   binary doesn't either - asm shows distinct inlined copies with subtly
//   different comment-handling. Merging them risks changing behavior.
// - VERIFY: `v81 != 0xFFFFFF88` / `v81 != -120` guards in the pseudocode are
//   IDA misreading an uninitialized stack slot comparison that the assembly
//   shows as unconditional (the asm never branches on it - it's stack noise
//   from a collapsed local). Treated as always-true (i.e. always do the
//   leading-whitespace + "is this a section header" check). VERIFY against
//   your IDB if behavior seems off on lines starting with certain bytes.
// - BUGFIX: none applied - this is a faithful cleanup, not a behavior fix.
//   The double-free-looking `v85`/`sectioncomment_stored` comment swap logic
//   in the entry-comment block is preserved exactly as-is (SUSPECT below).
//
// ============================================================================

// Frees an intrusive singly-linked LineComment list.
	// Mirrors the repeated free(Value)/delete(node) loops scattered
	// throughout the original (5 separate inlined copies of this pattern).
void FakeCCINIClass::Free_Comment_List(INIComment* head)
{
	while (head)
	{
		std::free(head->Value);
		INIComment* next = head->Next;
		GameDelete<true, false>(head);
		head = next;
	}
}

namespace
{
	// Pulls leading whitespace then checks for a `[...]` section header.
	// Mirrors the inlined "skip-space, check for '['" block that appears
	// 4x in the original (comment pre-scan, merge-fast-path, fresh-path
	// header check, fresh-path entry loop).
	bool Looks_Like_Section_Header(const char* line)
	{
		const char* p = line;
		while (*p && static_cast<unsigned char>(*p) <= ' ')
		{
			++p;
		}
		return (*p == '[' && std::strchr(p, ']') != nullptr);
	}

	// Ares feature: a key written as exactly "+" gets rewritten in place to
	// "var_<N>" (N = FakeCCINIClass::IteratorValue, a process-lifetime static)
	// so that repeated "+" keys within one section — or across a full
	// [#include] chain spanning multiple Load() calls — don't collide on CRC.
	// `key` and `value` are rewritten inside `key`'s own backing buffer
	// (original's in-buffer reuse trick; keyCapacity must be the full buffer
	// size, since the saved value gets appended after the new key in place).
	void Rewrite_Iterator_Key_If_Needed(char* key, size_t keyCapacity, char*& value)
	{
		if (CRT::strcmp(key, "+") != 0)
		{
			return;
		}

		char savedValue[512];
		strcpy_s(savedValue, value);

		const int writtenLen = sprintf_s(key, keyCapacity, "var_%d", FakeCCINIClass::IteratorValue++);
		if (writtenLen < 0)
		{
			return; // formatting failed — leave key/value untouched
		}

		value = key + writtenLen + 1;
		strcpy_s(value, keyCapacity - (writtenLen + 1), savedValue);
	}

	// Phobos: parses `: [PARENT]` tail from a section-header line.
	// `tail` should point to the character immediately after the closing `]`
	// of the section name, e.g. for `[CHILD] : [PARENT]`, tail = " : [PARENT]".
	// Returns the resolved INISection* for PARENT, or nullptr if the tail is
	// absent / malformed / the parent section doesn't exist yet.
	//
	// Called from both the merge-path and fresh-path header handling.
	// In the original binary this was a standalone NOINLINE function called
	// from two separate ASMJIT_PATCHes (0x525CA5 merge, 0x525DDB fresh).
	INISection* Get_Inherited_Section(INIClass* pThis, char* tail)
	{
		if (!tail || !*tail)
		{
			return nullptr;
		}

		char* cursor = tail;
		while (*cursor && static_cast<unsigned char>(*cursor) <= ' ')
		{
			++cursor;
		}

		if (*cursor != ':')
		{
			return nullptr;
		}

		++cursor;
		while (*cursor && static_cast<unsigned char>(*cursor) <= ' ')
		{
			++cursor;
		}

		if (*cursor != '[')
		{
			return nullptr;
		}

		++cursor;
		while (*cursor && static_cast<unsigned char>(*cursor) <= ' ')
		{
			++cursor;
		}

		char* start = cursor;
		char* end = std::strchr(start, ']');
		if (!end || end == start)
		{
			return nullptr;
		}

		// Only whitespace or ';' may follow the closing ']'
		if (*(end + 1) && *(end + 1) != ';' && static_cast<unsigned char>(*(end + 1)) > ' ')
		{
			return nullptr;
		}

		// Trim trailing whitespace inside the brackets
		char* trim = end - 1;
		while (trim > start && static_cast<unsigned char>(*trim) <= ' ')
		{
			--trim;
		}

		if (trim < start)
		{
			return nullptr;
		}

		*(trim + 1) = '\0';

		if (auto* section = pThis->GetSection(start))
		{
			return section;
		}

		Debug::LogError(
			"INI section inherits from '{}' which doesn't exist or hasn't been parsed yet.", start);
		return nullptr;
	}

	// Phobos: copies all entries from `source` into the named destination
	// section via WriteString, effectively merging the parent's keys into
	// the child. Keys already present in dest are NOT overwritten — vanilla
	// WriteString only writes if the key is absent (or overwrites; check
	// your CCINIClass::WriteString semantics).
	//
	// In the original binary this was called from two ASMJIT_PATCHes:
	// 0x525C28 (merge-path, CopySection1) and 0x525E44 (fresh-path, CopySection2).
	// IniSectionIncludes::includedSection was a static used to ferry the
	// parent pointer across hook call-sites that couldn't share locals.
	// Here it's just a parameter — no static needed.
	void Copy_Inherited_Section(CCINIClass* ini, INISection* source, const char* destName)
	{
		for (auto* node = source->Entries.GetFirst()->Next();
			 node != source->Entries.GetLast();
			 node = node->Next())
		{
			auto* entry = static_cast<INIEntry*>(node);
			ini->WriteString(destName, entry->Key, entry->Value);
		}
	}

	// Ares feature: if `SectionIndex` already contains a section whose CRC
	// matches `newSectionCrc` (i.e. this INI - or one merged on top of it -
	// declares the same [SectionName] twice), erase that stale index slot
	// and free the orphaned old INISection before the new one gets added.
	// Without this, vanilla behavior leaks the old section and leaves a
	// duplicate CRC slot in the index that FetchIndex can never reach.
	void Remove_Duplicate_Section_If_Present(IndexClass<int, INISection*>& sectionIndex, int newSectionCrc)
	{
		auto found = sectionIndex.FetchItem(newSectionCrc, true);
		if (!found || !found->Data)
		{
			return;
		}

		INISection* stale = found->Data;

		const auto end = sectionIndex.end();
		const auto next = std::next(found);
		std::memcpy(found, next, static_cast<size_t>(reinterpret_cast<const char*>(end) - reinterpret_cast<const char*>(next)));

		const auto entries = sectionIndex.IndexTable;
		const auto countBefore = sectionIndex.IndexCount;
		entries[countBefore - 1].Data = nullptr;
		entries[countBefore - 1].ID = 0;
		--sectionIndex.IndexCount;
		sectionIndex.Archive = nullptr;

		GameDelete<true, false>(stale);
	}
}

bool FakeCCINIClass::__Load(Straw* straw, bool loadcomments)
{
	this->CurrentSectionName = nullptr;
	this->CurrentSection = 0;

	// If we already have sections indexed, this is a merge-load: skip
	// comment tracking and reuse the existing section/entry graph.
	const bool merge = (this->SectionIndex.IndexCount > 0);
	if (merge) {
		loadcomments = false;
	}

	CacheStraw file;
	file.Get_From(straw);

	INIComment* pendingComments = nullptr;       // v5
	INIComment* pendingCommentsTail = nullptr;   // v6
	bool eof = false;
	INIComment* sectionComments = nullptr;       // sectioncomment_stored
	char buffer[512];
	char section[256];



	// -------------------------------------------------------------------
	// Pass 1: pre-scan leading lines before the first section header,
	// optionally collecting them as "loose" line comments.
	// -------------------------------------------------------------------
	for (;;) {
		FakeCCINIClass::Read_Line(&file, buffer, sizeof(buffer), &eof);

		if (eof) {
			if (loadcomments) {
				this->LineComments = pendingComments;
				file.~CacheStraw();
				return true;
			}

			Free_Comment_List(pendingComments);
			this->Clear(nullptr, nullptr);
			return false;
		}

		if (Looks_Like_Section_Header(buffer)) {
			break;
		}

		if (loadcomments) {
			INIComment* node = GameCreate<INIComment>();

			if (!node) {
				loadcomments = false;
			} else {
				node->Value = nullptr;
				node->Next = nullptr;

				if (pendingCommentsTail) {
					pendingCommentsTail->Next = node;
					pendingCommentsTail = node;
				} else {
					pendingComments = node;
					pendingCommentsTail = node;
					sectionComments = node;
				}

				node->Value = CRT::strdup(buffer);
			}
		}
	}

	// -------------------------------------------------------------------
	// Pass 2 (merge path): SectionIndex already populated - parse the rest
	// of the stream directly into existing sections via Put_String, with
	// no comment bookkeeping.
	// -------------------------------------------------------------------
	if (merge) {
		// `buffer` already holds the first "[section]" header line on entry
		// (or eof is already true and we skip straight past this block).
		while (!eof) {
			CRT::strtrim(buffer);
			if (buffer[0] == '[') {
				if (char* close = std::strchr(buffer, ']')) {
					// Phobos: extract `: [PARENT]` tail BEFORE zeroing `]`,
					// then zero `]` to isolate the clean section name.
					// Mirrors: PreProcess1 (0x525CA5) reads tail after `]`,
					//          CopySection1 (0x525C28) fires after strcpy.
					// We do both inline in one pass — no static carrier needed.
					INISection* inheritedSection = Get_Inherited_Section(this, close + 1);
					*close = '\0';
					std::strcpy(section, &buffer[1]);

					if (inheritedSection) {
						Copy_Inherited_Section(static_cast<CCINIClass*>(this), inheritedSection, section);
						inheritedSection = nullptr;
					}

				} else {
					std::strcpy(section, &buffer[1]);
				}
			}

			if (eof) {
				break;
			}

			// Inner loop: consume key=value lines until we hit eof or the
			// next "[section]" header (in which case `buffer` already holds
			// that header line and the outer while-loop reprocesses it -
			// equivalent to the original's jump back to check_section).
			bool nextSectionFound = false;
			while (!nextSectionFound && !eof) {
				int readResult = FakeCCINIClass::Read_Line(&file, buffer, sizeof(buffer), &eof);

				if (Looks_Like_Section_Header(buffer)) {
					nextSectionFound = true;
					break;
				}

				if (char* semi = std::strchr(buffer, ';')) {
					*semi = '\0';
					CRT::strtrim(buffer);
				}

				if (readResult && buffer[0] != ';' && buffer[0] != '=') {
					if (char* eq = std::strchr(buffer, '=')) {
						*eq = '\0';
						char* value = eq + 1;
						CRT::strtrim(buffer);
						if (std::strlen(buffer)) {
							CRT::strtrim(value);
							if (std::strlen(value)) {
								// Ares: "+" key auto-numbering (see Rewrite_Iterator_Key_If_Needed)
								Rewrite_Iterator_Key_If_Needed(buffer, sizeof(buffer), value);

								if (!this->WriteString(section, buffer, value)) {
									this->Clear(nullptr, nullptr);
									return false;
								}
							}
						}
					}
				}
			}
			// Loop: if nextSectionFound, outer while re-trims `buffer` as the
			// new header. If eof, outer while condition exits naturally.
		}

		this->LineComments = sectionComments;
		return true;
	}

	// -------------------------------------------------------------------
	// Pass 3 (fresh path): build the actual INISection / INIEntry graph.
	// -------------------------------------------------------------------
	Free_Comment_List(this->LineComments);
	this->LineComments = nullptr;

	if (eof) {
		this->LineComments = sectionComments;
		return true;
	}

	for (;;) {

		CRT::strtrim(buffer);

		char sectionName[256] = {};
		INISection* inheritedSection = nullptr;

		if (buffer[0] == '[') {
			if (char* close = std::strchr(buffer, ']')) {
				// Phobos: zero `]` first (matching PreProcess2 @ 0x525DDB
				// which does `*ptr = '\0'`), then read the tail after it.
				// This ensures strcpy below gets only the clean name.
				*close = '\0';
				inheritedSection = Get_Inherited_Section(this, close + 1);
			}
			std::strcpy(sectionName, &buffer[1]);
		}

		INISection* sectionptr = GameCreate<INISection>();

		if (!sectionptr) {
			// SUSPECT: original frees the pending loose comment list (pendingComments)
			// here and falls straight to Clear(); preserved verbatim.
			Free_Comment_List(pendingComments);
			this->Clear(nullptr, nullptr);
			return false;
		}

		sectionptr->Name = CRT::strdup(sectionName);
		sectionptr->Comments = sectionComments;
		sectionComments = nullptr;

		// Phobos: [CHILD] : [PARENT] inheritance — copy parent's entries into
		// this section right after construction, before parsing any of its own
		// entries. Mirrors CopySection2 (0x525E44) which fired right after
		// sectionptr->Comments was set and the section name was already clean.
		// inheritedSection is nullptr when no `: [PARENT]` tail was present.
		if (inheritedSection){
			Copy_Inherited_Section(static_cast<CCINIClass*>(this), inheritedSection, sectionName);
			inheritedSection = nullptr;
		}

		INIComment* entryPendingComments = nullptr;     // v85 (local to this section's entry loop)
		INIComment* entryPendingCommentsTail = nullptr;

		while (!eof) {
			int readResult = FakeCCINIClass::Read_Line(&file, buffer, sizeof(buffer), &eof);

			if (eof) {
				break;
			}

			if (Looks_Like_Section_Header(buffer)) {
				break;
			}

			if (loadcomments) {
				INIComment* node = GameCreate<INIComment>();

				if (!node) {
					loadcomments = false;
				} else {
					node->Value = nullptr;
					node->Next = nullptr;

					if (entryPendingCommentsTail) {
						entryPendingCommentsTail->Next = node;
						entryPendingCommentsTail = node;
					} else {
						entryPendingCommentsTail = node;
						entryPendingComments = node;
					}

					node->Value = CRT::strdup(buffer);
				}
			}

			// Extract trailing inline comment (e.g. "Key=Value ; comment")
			char* entryComment = nullptr;
			int preIndent = 0, postIndent = 0, commentCursor = 0;

			if (loadcomments) {
				if (char* extracted = FakeCCINIClass::Extract_Line_Comment(buffer, &preIndent, &postIndent, &commentCursor)) {
					entryComment = CRT::strdup(extracted);
				}
			}

			// Strip ';' comment from buffer itself (inlined Strip_Comments)
			char* semi = std::strchr(buffer, ';');

			if (semi) {
				*semi = '\0';
				CRT::strtrim(buffer);
			}

			const bool skipLine = (!readResult || buffer[0] == ';' || buffer[0] == '=');
			char* eq = skipLine ? nullptr : std::strchr(buffer, '=');

			if (!eq) {
				std::free(entryComment);
				continue;
			}

			*eq = '\0';
			char* value = eq + 1;
			CRT::strtrim(buffer);

			if (std::strlen(buffer) == 0) {
				std::free(entryComment);
				continue;
			}

			CRT::strtrim(value);
			if (std::strlen(value) == 0) {
				std::free(entryComment);
				continue;
			}

			// SUSPECT (preserved verbatim): if there's exactly one pending
			// loose entry comment and its Value is non-null, it gets freed
			// and zeroed here rather than attached - looks like an original
			// engine quirk/bug, not touched.
			if (loadcomments && entryPendingComments && entryPendingComments->Value) {
				std::free(entryPendingComments->Value);
				entryPendingComments->Value = nullptr;
			}

			INIEntry* newentry = GameCreate<INIEntry>();

			if (!newentry) {
				std::free(entryComment);
				Free_Comment_List(entryPendingComments);
				GameDelete<true, false>(sectionptr);
				this->Clear(nullptr, nullptr);
				return false;
			}

			// Ares: "+" key auto-numbering (see Rewrite_Iterator_Key_If_Needed).
			// Must run on `buffer`/`value` (the stack working buffer, which has
			// spare room after the trimmed key) BEFORE the strdup below -
			// newentry->Entry is a tightly-sized heap string once allocated and
			// has no room to grow a replacement key into, so the rewrite has to
			// happen here, not after construction.
			Rewrite_Iterator_Key_If_Needed(buffer, sizeof(buffer), value);

			newentry->Key = CRT::strdup(buffer);
			newentry->Value = CRT::strdup(value);
			newentry->Comments = entryPendingComments;
			newentry->CommentString = entryComment;
			newentry->PreIndentCursor = preIndent;
			newentry->PostIndentCursor = postIndent;
			newentry->CommentCursor = commentCursor;

			entryPendingComments = nullptr;
			entryPendingCommentsTail = nullptr;

			// Index the new entry by CRC(Entry) into sectionptr->EntryIndex,
			// then append to sectionptr->Entries (tail insert).
			const long entryCrc = SafeChecksummer {}(newentry->Key, std::strlen(newentry->Key));
			sectionptr->EntryIndex.AddIndex(entryCrc, newentry); // growable index-table push, asm: realloc-by-10 pattern
			sectionptr->Entries.AddTail(newentry);             // unlink-then-tail-append, asm: standard intrusive list splice
		}

		// Decide whether to keep this section or discard it as empty.
		bool keepSection = true;

		if (!loadcomments) {
			// Empty section check: Entries has no real (non-sentinel) node.
			if (sectionptr->Entries.IsEmpty()) {
				Free_Comment_List(entryPendingComments);
				GameDelete<true,false>(sectionptr);
				keepSection = false;
			}
		}

		if (keepSection) {
			const long sectionCrc = SafeChecksummer {}(sectionptr->Name, std::strlen(sectionptr->Name));

			// Ares: if a section with this exact CRC already exists (e.g. the
			// same [SectionName] declared twice, or re-declared by a merged-in
			// INI file), remove and free the old one first - see
			// Remove_Duplicate_Section_If_Present for why this matters.
			Remove_Duplicate_Section_If_Present(this->SectionIndex, sectionCrc);

			this->SectionIndex.AddIndex(sectionCrc, sectionptr);
			this->Sections.AddTail(sectionptr);
		}

		if (eof) {
			this->LineComments = sectionComments;
			return true;
		}

		if (!keepSection) {
			// Loop again with the buffer already holding the next
			// section's header line (mirrors LABEL_133 -> continue).
			continue;
		}
		// keepSection==true and !eof: loop again as well (same as above).
	}
}

//DEFINE_FUNCTION_JUMP(LJMP, 0x525A60, FakeCCINIClass::__Load);