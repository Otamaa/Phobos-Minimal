#pragma once

/**
 *  Creates a meteor shower around the current mouse cell.
 */
class MeteorShowerCommandClass : public ViniferaCommandClass
{
public:
	MeteorShowerCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
	virtual ~MeteorShowerCommandClass() { }

	virtual const char* Get_Name() const override;
	virtual const char* Get_UI_Name() const override;
	virtual const char* Get_Category() const override;
	virtual const char* Get_Description() const override;
	virtual bool Process() override;

	virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
};


/**
 *  Sends a meteor at the current mouse cell.
 */
class MeteorImpactCommandClass : public ViniferaCommandClass
{
public:
	MeteorImpactCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
	virtual ~MeteorImpactCommandClass() { }

	virtual const char* Get_Name() const override;
	virtual const char* Get_UI_Name() const override;
	virtual const char* Get_Category() const override;
	virtual const char* Get_Description() const override;
	virtual bool Process() override;

	virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
};