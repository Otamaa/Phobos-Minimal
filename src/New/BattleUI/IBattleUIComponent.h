#pragma once

#include <Point2D.h>

struct IStream;


/**
 *  Lifecycle interface for battle UI components (sidebar, radar, etc.).
 */
class IBattleUIComponent
{
public:
    virtual ~IBattleUIComponent() = default;

    virtual void One_Time() = 0;
    virtual void Init_Clear() = 0;
    virtual void Init_IO() = 0;
    virtual void Init_For_House() = 0;
    virtual void AI(const int& key, Point2D &mouse) = 0;
    virtual void Draw() = 0;
    virtual void Blit(bool complete) = 0;
    virtual void Shift_Sidebar() = 0;

    virtual const wchar_t* GetToolTip(int gadget_id) { return nullptr; }

    virtual HRESULT Save(IStream *pStm) const { return S_OK; }
    virtual HRESULT Load(IStream *pStm) { return S_OK; }
};
