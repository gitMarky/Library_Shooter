
static const POSE_STANDING = "Standing";
static const POSE_CROUCHING = "Crouching";
static const POSE_PRONE = "Prone";

static const WEAPON_IDLE = "WeaponIdle";
static const WEAPON_READY = "WeaponReady";
static const WEAPON_AIMING = "WeaponAiming";

func InitializePlayer(int plr)
{
	// Set zoom to full map size.
	SetPlayerZoomByViewRange(plr, 300, nil, PLRZOOM_Direct);

	// No FoW to see everything happening.
	SetFoW(false, plr);

	// Start!
	LaunchTest(1);
	return;
}

global func CreateStanceManager()
{
	// Standing state
	var standing = new StanceDefinition { Name = POSE_STANDING, };
	var crouching = new StanceDefinition { Name = POSE_CROUCHING, };
	var prone = new StanceDefinition { Name = POSE_PRONE, };
	
	// Define transitions
	standing->AddTransition(crouching)
	        ->AddTransition(prone)
	        ->AddTransition(crouching)
	        ->AddTransition(standing);
	        
	// Aiming state
	var weapon_idle = new StanceDefinition { Name = WEAPON_IDLE, };
	var weapon_ready = new StanceDefinition { Name = WEAPON_READY, };
	var weapon_aiming = new StanceDefinition { Name = WEAPON_AIMING, };
	
	weapon_idle->AddTransition(weapon_ready)->AddTransition(weapon_idle);
	weapon_ready->AddTransition(weapon_aiming)->AddTransition(weapon_ready);

	// Add everything to the stance maanger

	var manager = CreateObject(Library_StanceManager);
	
	// So that it can be retrieved via string
	manager->AddStance(standing, crouching, prone, weapon_idle, weapon_ready, weapon_aiming);

	// Initial state
	manager->SetStance(standing);
	manager->SetStance(weapon_idle, 1);
	return manager;
}

global func doTestTransition(object manager, any channel, string from, string to, bool result, bool forced)
{
	var desc = Format("[%s => %s](%v): ", from, to, channel);

	var final;
	if (result || forced)
	{
		final = to;
	}
	else
	{
		final = from;
	}

	doTest(Format("%s%s", desc, "Initial stance should be \"%s\", got \"%s\""), from, manager->GetStance(channel).Name);
	doTest(Format("%s%s", desc, "Transition should return %v, got %v"), result, manager->SetStance(to, channel, forced));
	doTest(Format("%s%s", desc, "Final stance should be \"%s\", got \"%s\""), final, manager->GetStance(channel).Name);
}


/* --- The actual tests --- */


// --------------------------------------------------------------------------------------------------------

global func Test1_OnStart() { return true; }
global func Test1_Execute()
{
	var manager = CreateStanceManager();
	
	Log("Stance manager tracks stances correctly in GetStance()");

	doTest("Stance for channel(nil) should be \"%s\", got \"%s\"", POSE_STANDING, manager->GetStance().Name);
	doTest("Stance for channel(0) should be \"%s\", got \"%s\"", POSE_STANDING, manager->GetStance(0).Name);
	doTest("Stance for channel(1) should be \"%s\", got \"%s\"", WEAPON_IDLE, manager->GetStance(1).Name);

	return Evaluate();
}

// --------------------------------------------------------------------------------------------------------

global func Test2_OnStart() { return true; }
global func Test2_Execute()
{
	var manager = CreateStanceManager();
	var channel = 0;

	Log("SetStance() works correctly for the known transitions, channel %d", channel);

	// Standing
	doTestTransition(manager, channel, POSE_STANDING, POSE_STANDING, false);
	doTestTransition(manager, channel, POSE_STANDING, POSE_PRONE, false);
	doTestTransition(manager, channel, POSE_STANDING, POSE_CROUCHING, true);

	// Crouching
	doTestTransition(manager, channel, POSE_CROUCHING, POSE_CROUCHING, false);
	doTestTransition(manager, channel, POSE_CROUCHING, POSE_PRONE, true);

	// Prone
	doTestTransition(manager, channel, POSE_PRONE, POSE_PRONE, false);
	doTestTransition(manager, channel, POSE_PRONE, POSE_STANDING, false);

	// Back
	doTestTransition(manager, channel, POSE_PRONE, POSE_CROUCHING, true);
	doTestTransition(manager, channel, POSE_CROUCHING, POSE_STANDING, true);

	return Evaluate();
}


// --------------------------------------------------------------------------------------------------------

global func Test3_OnStart() { return true; }
global func Test3_Execute()
{
	var manager = CreateStanceManager();
	var channel = 1;

	Log("SetStance() works correctly for the known transitions, channel %d", channel);

	// Idle
	doTestTransition(manager, channel, WEAPON_IDLE, WEAPON_IDLE, false);
	doTestTransition(manager, channel, WEAPON_IDLE, WEAPON_AIMING, false);
	doTestTransition(manager, channel, WEAPON_IDLE, WEAPON_READY, true);

	// Ready
	doTestTransition(manager, channel, WEAPON_READY, WEAPON_READY, false);
	doTestTransition(manager, channel, WEAPON_READY, WEAPON_AIMING, true);

	// Aiming
	doTestTransition(manager, channel, WEAPON_AIMING, WEAPON_AIMING, false);
	doTestTransition(manager, channel, WEAPON_AIMING, WEAPON_IDLE, false);

	// Back
	doTestTransition(manager, channel, WEAPON_AIMING, WEAPON_READY, true);
	doTestTransition(manager, channel, WEAPON_READY, WEAPON_IDLE, true);

	return Evaluate();
}


// --------------------------------------------------------------------------------------------------------

global func Test4_OnStart() { return true; }
global func Test4_Execute()
{
	var manager = CreateStanceManager();
	var channel = 1;

	Log("SetStance() does not accept invalid transitions", channel);

	doTestTransition(manager, 0, POSE_STANDING, WEAPON_IDLE, false);
	doTestTransition(manager, 0, POSE_STANDING, WEAPON_AIMING, false);
	doTestTransition(manager, 0, POSE_STANDING, WEAPON_READY, false);

	return Evaluate();
}


// --------------------------------------------------------------------------------------------------------

global func Test5_OnStart() { return true; }
global func Test5_Execute()
{
	var manager = CreateStanceManager();
	var channel = 0;

	Log("SetStance() with forced parameter works for any transitions, channel %d", channel);

	doTestTransition(manager, channel, POSE_STANDING, POSE_STANDING, false, true);
	doTestTransition(manager, channel, POSE_STANDING, POSE_PRONE, true, true);
	doTestTransition(manager, channel, POSE_PRONE, POSE_PRONE, false, true);
	doTestTransition(manager, channel, POSE_PRONE, POSE_STANDING, true, true);
	doTestTransition(manager, channel, POSE_STANDING, WEAPON_AIMING, true, true);

	return Evaluate();
}
