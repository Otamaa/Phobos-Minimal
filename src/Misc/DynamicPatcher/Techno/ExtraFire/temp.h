// parse primary and secondary
for (int i = 0; i < 2; ++i)
{
	if (i > 0)
		std::strcpy(prefixBuffer, "Secondary");
	else
		std::strcpy(prefixBuffer, "Primary");

	_snprintf_s(tempBuffer_fix, sizeof(tempBuffer_fix), "ExtraFire.%s", prefixBuffer);

	ValueableVector<WeaponTypeClass*> buffer_Weapon;
	buffer_Weapon.Read(parserRules, pSection_rules, tempBuffer_fix);

	if (!buffer_Weapon.empty())
	{
		Nullable<CoordStruct> buffer_FLH;
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.FLH", tempBuffer_fix);
		buffer_FLH.Read(parserArt, pSection_Art, tempBuffer);

		Debug::Log("Reading ExtraFireData found for T[%s] Tag[%s] ! \n", pTech->get_ID(), tempBuffer_fix);

		_snprintf_s(tempBuffer_fix, sizeof(tempBuffer_fix), "ExtraFire.Elite%s", prefixBuffer);

		ValueableVector<WeaponTypeClass*> buffer_Weapon_Elite;
		buffer_Weapon_Elite.Read(parserRules, pSection_rules, tempBuffer_fix);

		if (buffer_Weapon_Elite.empty())
			buffer_Weapon_Elite = buffer_Weapon;

		Nullable<CoordStruct> buffer_FLH_Elite;
		_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.FLH", tempBuffer_fix);
		buffer_FLH_Elite.Read(parserArt, pSection_Art, tempBuffer);

		if (!buffer_FLH_Elite.isset() || buffer_FLH_Elite.Get() == CoordStruct::Empty)
			buffer_FLH_Elite = buffer_FLH.Get();



		WeaponFLHData.emplace_back(std::move(buffer_FLH.Get(CoordStruct::Empty)));

		EliteWeaponData.push_back(std::move(buffer_Weapon_Elite));
		EliteWeaponFLHData.emplace_back(std::move(buffer_FLH_Elite.Get()));
		++sizetotal;
	}

}

if (pTech->WeaponCount && (pTech->WeaponCount > 0))
{
	for (int b = 0; b < pTech->WeaponCount; ++b)
	{
		_snprintf_s(tempBuffer_fix, sizeof(tempBuffer_fix), "ExtraFire.Weapon%d", b + 1);

		ValueableVector<WeaponTypeClass*> buffer_Weapon;
		buffer_Weapon.Read(parserRules, pSection_rules, tempBuffer_fix);

		if (!buffer_Weapon.empty())
		{
			Nullable<CoordStruct> buffer_FLH;
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.FLH", tempBuffer_fix);
			buffer_FLH.Read(parserArt, pSection_Art, tempBuffer);

			Debug::Log("Reading ExtraFireData found for T[%s] Tag[%s] ! \n", pTech->get_ID(), tempBuffer_fix);

			_snprintf_s(tempBuffer_fix, sizeof(tempBuffer_fix), "ExtraFire.EliteWeapon%d", b + 1);

			ValueableVector<WeaponTypeClass*> buffer_Weapon_Elite;
			buffer_Weapon_Elite.Read(parserRules, pSection_rules, tempBuffer_fix);

			if (buffer_Weapon_Elite.empty())
				buffer_Weapon_Elite = buffer_Weapon;

			Nullable<CoordStruct> buffer_FLH_Elite;
			_snprintf_s(tempBuffer, sizeof(tempBuffer), "%s.FLH", tempBuffer_fix);
			buffer_FLH_Elite.Read(parserArt, pSection_Art, tempBuffer);

			if (!buffer_FLH_Elite.isset() || buffer_FLH_Elite.Get() == CoordStruct::Empty)
				buffer_FLH_Elite = buffer_FLH.Get();

			WeaponData.emplace_back(std::move(buffer_Weapon));
			WeaponFLHData.emplace_back(std::move(buffer_FLH.Get(CoordStruct::Empty)));
			EliteWeaponData.push_back(std::move(buffer_Weapon_Elite));
			EliteWeaponFLHData.emplace_back(std::move(buffer_FLH_Elite.Get()));
			++sizetotal;
		}
	}

	if (sizetotal > 0)
	{
		WeaponFLHData.resize(sizetotal);
		EliteWeaponFLHData.resize(sizetotal);
		WeaponData.resize(sizetotal);
		EliteWeaponData.resize(sizetotal);
	}
	else
	{
		WeaponFLHData.clear();
		EliteWeaponFLHData.clear();
		WeaponData.clear();
		EliteWeaponData.clear();
	}
}
