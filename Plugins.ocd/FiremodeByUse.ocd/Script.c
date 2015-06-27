
static const WEAPON_Firemode_Primary = "primary";
static const WEAPON_Firemode_Secondary = "secondary";

local selected_firemode;

/**
 Callback: the current firemode. Overload this function for
 @return proplist The current firemode.
 @version 0.1.0
 @par user The object that is using the weapon.
 @par x The x coordinate the user is aiming at.
 @par y The y coordinate the user is aimint at.
 @version 0.1.0
 */
public func GetFiremode()
{
	return GetProperty(selected_firemode, this.fire_modes);
}

/**
 Callback: Pressed the regular use button (fire).
 */
public func OnPressUse(object user, int x, int y)
{
	if (this->~CanChangeFiremode())
	{
		ChangeFiremode(WEAPON_Firemode_Primary);
	}
}

/**
 Callback: Pressed the alternate use button (fire secondary).
 @par user The object that is using the weapon.
 @par x The x coordinate the user is aiming at.
 @par y The y coordinate the user is aimint at.
 @version 0.1.0
 */
public func OnPressUseAlt(object user, int x, int y)
{
	if (this->~CanChangeFiremode())
	{
		ChangeFiremode(WEAPON_Firemode_Secondary);
	}
}


/**
 Use this to change the firemode of the weapon.
 @par firemode The name of the new firemode.
 @version 0.1.0
 */
public func ChangeFiremode(string firemode)
{
	if (firemode == nil)
	{
		FatalError("The function expects a fire mode that is not nil");
	}

	selected_firemode = firemode;
}