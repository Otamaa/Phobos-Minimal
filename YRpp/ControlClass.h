#pragma once

#include <GadgetClass.h>

class NOVTABLE ControlClass : public GadgetClass
{
public:

	//Destructor
	virtual ~ControlClass() RX;

	//GadgetClass
	virtual unsigned int const GetID() override JMP_THIS(0x48E610);
	virtual bool Draw(bool forced) override JMP_THIS(0x48E620);
	virtual bool Action(GadgetFlag flags, WWKey* pKey, KeyModifier modifier) override JMP_THIS(0x48E5A0);
	virtual ControlClass* ExtractGadget(unsigned int nID) override R0;
	virtual void PeerToPeer(unsigned int flags, WWKey* pKey, GadgetClass* pSendTo) override RX;

	//ControlClass
	virtual void MakePeer(ControlClass const& target) RX;

	//Non virtual

	//Statics

	//Constructors
	ControlClass(unsigned int nID, int nX, int nY, int nWidth, int nHeight, GadgetFlag eFlag, bool bSticky) noexcept
		: ControlClass(noinit_t()) { JMP_THIS(0x48E520); }

	ControlClass(unsigned int nID, RectangleStruct rect, GadgetFlag eFlag, bool bSticky) noexcept
		: ControlClass(noinit_t()) { JMP_THIS(0x48E520); }

	ControlClass(ControlClass& nAnother) noexcept
		: ControlClass(noinit_t()) { JMP_THIS(0x48E570); }

protected:
	explicit __forceinline ControlClass(noinit_t)  noexcept
		: GadgetClass(noinit_t())
	{
	}

	//Properties
public:

	int ID;
	GadgetClass* SendTo; // Peer
};

static_assert(sizeof(ControlClass) == 0x2C, "Invalid Size !");