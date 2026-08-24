#pragma once

class PhobosStreamReader;
class PhobosStreamWriter;
// Container for AttachEffect attachment for an individual effect passed to AE attach function.
struct AEAttachParams
{
	int DurationOverride { 0 };
	int Delay { 0 };
	int InitialDelay { 0 };
	int RecreationDelay { -1 };
	int CumulativeSourceMaxCount { -1 };
	bool CumulativeRefreshAll { false };
	bool CumulativeRefreshAll_OnAttach { false };
	bool CumulativeRefreshSameSourceOnly { true };

public :

	bool Load(PhobosStreamReader& stm, bool registerForChange);
	bool Save(PhobosStreamWriter& stm) const;

private:
	template <typename T>
	bool Serialize(T& stm);
};