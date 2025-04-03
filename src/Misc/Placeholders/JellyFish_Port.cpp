
/*
	Main JellyFish Port
	- UnitClass::Fire -> commanding the locomotor to do 0x8C if return false return the `TechnoClass::CanFire` result ??
	- UnitClass::Can_enter_cell -> 0x73FC43 , check if Jellyfish then return 73FD37
	- UnitClass::Can_enter_cell -> 0x73FAEF , check if Jellyfish first , if yes 0x73FA7C , if no continue to check the value
	- TS function 0x653090 ???
	- UnitClass::Unlibo -> if JellyFish set the stage to 0 , set the rate to 1 , set DelayTime to 1
	- TS function 0x64F020 , Jellyfish update func
	- UnitClass::Approach_Target , IF Unit && if Jellyfish -> return
	- TS ExplosionDamage func -> 0x45F22B ??
*/
