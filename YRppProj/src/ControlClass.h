#pragma once

#include <GadgetClass.h>

class NOVTABLE ControlClass : public GadgetClass
{
public:

	//Destructor
	virtual ~ControlClass() RX;

	//GadgetClass
	virtual unsigned int const GetID() override;//JMP_THIS(0x48E610);
	virtual bool Draw(bool forced) override;//JMP_THIS(0x48E620);
	virtual bool Action(GadgetFlag flags, DWORD* pKey, KeyModifier modifier) override;//JMP_THIS(0x48E5A0);
	virtual ControlClass* ExtractGadget(unsigned int nID) R0;
	virtual void PeerToPeer(unsigned int nFlags, DWORD* pKey, ControlClass* pSendTo) RX;

	//ControlClass
	virtual void MakePeer(ControlClass const& target) RX;
	virtual void SetSendTo(GadgetClass* pSendTo);// JMP_THIS(0x48E600);

	//Non virtual

	//Statics

	//Constructors
	ControlClass(unsigned int nID, int nX, int nY, int nWidth, int nHeight, GadgetFlag eFlag, bool bSticky);
	ControlClass(unsigned int nID, RectangleStruct rect, GadgetFlag eFlag, bool bSticky);
	ControlClass(ControlClass& nAnother);

	//Properties
public:

	int ID;
	GadgetClass* SendTo; // Peer
};

static_assert(sizeof(ControlClass) == 0x2C, "Invalid Size !");