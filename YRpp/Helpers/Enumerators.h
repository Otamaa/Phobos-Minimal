#pragma once

#include <FootClass.h>
#include <Helpers/Cast.h>

#include <cmath>

// Enumerates the objects in a linked list.
/*
	The next element is retrieved eagerly, thus this enumerator supports
	removing the current element from the list.

	\author AlexB
*/
template <typename T, T* T::* Next>
class ListEnumerator
{
public:
	ListEnumerator()
		: current(nullptr), next(nullptr)
	{ }

	ListEnumerator(T* first)
		: current(first), next(first ? first->*Next : nullptr)
	{ }

	explicit operator bool() const {
		return current != nullptr;
	}

	T* operator * () const {
		return current;
	}

	T* operator -> () const {
		return current;
	}

	ListEnumerator& operator ++ () {
		current = next;
		if(next) {
			next = next->*Next;
		}
		return *this;
	}

	ListEnumerator operator ++ (int) {
		auto const old = *this;
		++*this;
		return old;
	}

private:
	T* current;
	T* next;
};

template <typename T, T* T::* Prev>
class PrevListEnumerator
{
public:
	PrevListEnumerator()
		: current(nullptr), prev(nullptr)
	{
	}

	PrevListEnumerator(T* first)
		: current(first), prev(first ? first->*Prev : nullptr)
	{
	}

	explicit operator bool() const
	{
		return current != nullptr;
	}

	T* operator * () const
	{
		return current;
	}

	T* operator -> () const
	{
		return current;
	}

	PrevListEnumerator& operator ++ ()
	{
		current = prev;
		if (prev)
		{
			prev = prev->*Prev;
		}
		return *this;
	}

	PrevListEnumerator operator ++ (int)
	{
		auto const old = *this;
		++* this;
		return old;
	}

private:
	T* current;
	T* prev;
};

using NextObject = ListEnumerator<ObjectClass, &ObjectClass::NextObject>;
using NextTeamMember = ListEnumerator<FootClass, &FootClass::NextTeamMember>;
using NextSHP = ListEnumerator<SHPReference, &SHPReference::Next>;
using PrevSHP = PrevListEnumerator<SHPReference, & SHPReference::Prev >;
// Enumerates the cells in a rectangle.
/*
	Enumeration starts at the top left and iterates left to right first, then
	top down. Returned values are the final cell coords, not offsets.

	\author AlexB
*/
class CellRectEnumerator
{
	LTRBStruct bounds;
	CellStruct current;

public:
	CellRectEnumerator(LTRBStruct const bounds)
		: bounds(bounds), current()
	{
		current = CellStruct{
			static_cast<short>(bounds.Left),
			static_cast<short>(bounds.Top) };
	}

	operator bool() const {
		return current.Y <= bounds.Bottom;
	}

	const CellStruct& operator * () const {
		return current;
	}

	CellRectEnumerator& operator ++ () {
		this->next();
		return *this;
	}

	void operator ++ (int) {
		this->next();
	}

protected:
	void next() {
		// increase the x coordinate and, if it is outside the bounds,
		// go to the beginning of the next "line"
		++current.X;

		if(current.X > bounds.Right) {
			current.X = static_cast<short>(bounds.Left);
			++current.Y;
		}
	}
};

// Enumerates the cells in a circular area.
/*
	Enumeration order cannot be depended upon. Returned values are the final
	cell coords, not offsets.

	\author AlexB
*/
class CellRangeEnumerator
{
	CellRectEnumerator inner;
	CellStruct center;
	double radius_sqr;

public:
	CellRangeEnumerator(CellStruct const center, double const radius)
		:
			inner(convert(center, radius)) ,
			center(center),
			radius_sqr(radius * radius)
	{
		next();
	}

	operator bool() const {
		return inner != false;
	}

	const CellStruct& operator * () const {
		return *inner;
	}

	CellRangeEnumerator& operator ++ () {
		++inner;
		this->next();
		return *this;
	}

	void operator ++ (int) {
		++inner;
		this->next();
	}

protected:
	static LTRBStruct convert(CellStruct const center, double const radius) {
		auto const range = static_cast<int>(std::floor(radius + 0.99)) * 2 + 1;

		auto const topleft = Point2D{
			center.X - range / 2, center.Y - range / 2 };

		return LTRBStruct{
			topleft.X, topleft.Y, topleft.X + range, topleft.Y + range };
	}

	void next() {
		// advance until done or cell is in range. slow, but makes no
		// assumptions about order
		while(inner && center.DistanceFromSquared(*inner) > radius_sqr) {
			++inner;
		}
	}
};

// Enumerates the cell offsets using the cell spread logic.
/*
	Enumeration starts at the center. Returned values are to be considered
	offsets, so the center is the origin.

	Offsets are returned in unspecified order, but all cells of CellSpread N
	are guaranteed to be returned before enumerating cells of N+1. This means
	that this class can extend or replace the original CellSpread table, as
	long as a single mechanism is used to complete a CellSpread distance.

	The algorithm does not generate any cell offset not in the requested
	CellSpread range.

	The following properties might change in the future:

	Currently, the implementation matches the output order of the original
	CellSpread table for values less than 11, except for the offsets 2 and 4
	(which are swapped here).

	The enumeration stops at Max not because of technical reasons, but because
	at some point is gets unfeasible to enumerate cell contents like this. A
	CellSpread of 256 contains 175789 cells. Input like 1000 to mean full map
	effect is a bad way to do something, and people should be punished for it.

	\author AlexB
*/
template<short Max = std::numeric_limits<short>::max()>
class CellSpreadEnumerator
{
	CellStruct current; //4
	short spread; //4
	short curspread; // 4
	bool hasTwo; //2
	bool hadTwo; //2

public:

	CellSpreadEnumerator(short Spread, short start= 0) noexcept :
	current(start ? -1 : 0  , -start),
	spread(MinImpl(Spread, Max)),
	curspread(start),
	hasTwo(true),
	hadTwo(true) { }

	CellSpreadEnumerator() = default;
	~CellSpreadEnumerator() = default;

	COMPILETIMEEVAL operator bool () const {
		return curspread <= spread;
	}

	COMPILETIMEEVAL const CellStruct& operator * () const {
		return current;
	}

	COMPILETIMEEVAL CellSpreadEnumerator& operator ++ () {
		this->next();
		return *this;
	}

	COMPILETIMEEVAL const CellStruct* operator->() const
	{
		return &current;
	}

	COMPILETIMEEVAL void operator ++ (int) {
		this->next();
	}

	COMPILETIMEEVAL void clear() {
		this->current = CellStruct::Empty;
		this->spread = 0;
		this->curspread = 0;
		this->hasTwo = false;
		this->hadTwo = false;
	}

	COMPILETIMEEVAL void setSpread(short Spread, short start = 0) {
		this->spread = MinImpl(Spread, Max);
		this->reset(start);
	}

	COMPILETIMEEVAL size_t getCurSpread() const {
		return this->curspread;
	}
protected:
	COMPILETIMEEVAL void reset(short radius) {
		curspread = radius;
		current.X = radius ? -1 : 0;
		current.Y = -radius;
		hasTwo = true;
		hadTwo = true;
	}

	COMPILETIMEEVAL bool next() {
		// already done?
		if(!*this) {
			return false;
		}

		// center or top-right-most cell finishes this
		// round. move to the start of the next one.
		if(current.SimilarTo(CellStruct::Empty) || (current.X == 1 && current.Y == curspread)) {
			reset(curspread + 1);
			return *this;
		}

		// finish this line
		if(hasTwo) {
			current.X++;

			// quick way to finish first and last line
			hasTwo = (current.X == 0);
			return true;
		}

		// if we are on the left side, mirror to the
		// right side and restore hasTwo.
		if(current.X < 0) {
			current.X = -current.X;
			hasTwo = hadTwo;
			return true;
		}

		// move up to the next line
		current.Y++;

		// map upper part to lower part
		// this is reverted later
		bool changeSign = (current.Y > 0);
		if(changeSign) {
			current.Y = -current.Y;
		}

		// for each line up, go two steps left
		current.X = (curspread + current.Y) * -2 - 1;

		int diff = current.X - current.Y;
		hasTwo = diff >= 0;
		hadTwo = hasTwo;

		if(diff == -1) {
			// "clip" the one
			current.X++;
		} else if(diff < 0) {
			// get from other direction
			current.X = -current.Y / 2 - curspread;
		}

		// revert the sign change
		if(changeSign) {
			current.Y = -current.Y;
		}

		return true;
	}
};

// Enumerates the cells from start to end.
/*
	If start equals end, at last this cell is returned.

	\author AlexB
*/
class CellSequenceEnumerator
{
	CellStruct current;
	CellStruct end;
	CellStruct offset;
	CellStruct distance;
	int value;
	bool valid;

public:
	CellSequenceEnumerator(const CellStruct& start, const CellStruct& end) :
		current(start),
		end(end),
		valid(true)
	{
		auto const difference = end - start;

		offset = {
			static_cast<short>(Math::signum(difference.X)),
			static_cast<short>(Math::signum(difference.Y))
		};

		distance = { (short)(difference.X * offset.X * 2), (short)(difference.Y * offset.Y * 2) };

		if(distance.X > distance.Y) {
			value = distance.Y - distance.X / 2;
		} else {
			value = distance.X - distance.Y / 2;
		}
	}

	explicit operator bool () const {
		return valid;
	}

	const CellStruct& operator * () const {
		return current;
	}

	CellSequenceEnumerator& operator ++ () {
		next();
		return *this;
	}

	void operator ++ (int) {
		next();
	}

protected:
	void next()  {
		if(current.SimilarTo(end)) {
			valid = false;

		// slower path, only taken if necessary
		} else if(offset.X == 0 || offset.Y == 0) {
			// only one component changes
			current += offset;

		} else {
			// Bresenham algo, with changes to not move
			// diagonally (iow: two cells in one step)
			if(distance.X > distance.Y) {
				if(value < 0) {
					value += distance.Y;
					current.X += offset.X;
				} else {
					value -= distance.X;
					current.Y += offset.Y;
				}
			} else {
				if(value < 0) {
					value += distance.X;
					current.Y += offset.Y;
				} else {
					value -= distance.Y;
					current.X += offset.X;
				}
			}
		}
	}
};
