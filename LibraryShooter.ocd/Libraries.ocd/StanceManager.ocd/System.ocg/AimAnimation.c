/**
 * Applies an aim animation, as in the OpenClonk AimManager.
 */
static const StanceBehaviour_AimAnimation = new Global
{
	/**
	 * Constructor function.
	 *
	 * @par aim_animation see DefineAnimation
	 * @par anim_slot The animations are played in this animation slot. Defaults to CLONK_ANIM_SLOT_Arms.
	 * @return proplist the created behaviour.
	 */
	Create = func (any aim_animation, int anim_slot)
	{
		anim_slot = anim_slot ?? CLONK_ANIM_SLOT_Arms;
		var behaviour = new StanceBehaviour_AimAnimation
		{
			AnimSlot = anim_slot,         // Play animations in this slot
			AnimAim = aim_animation,      // Define the default aim animation
			AnimAimStatus = nil,          // Status for the loop animation: integer = start playing in that frame; nil = needs to play; bool = is playing
			AnimAimIndex = nil,           // Index of aim animation
			AnimAimSpeed = 1800,          // The aim angle can change by this many degrees * 0.1; default is 180 degrees = instant change
		};
		return behaviour->DefineAnimation(aim_animation);
	},


	/**
	 * Sets the speed at which the current aim angle can change,
	 * in 10ths of degrees per frame.
	 * A value of 0 or nil resets the speed to its default value:
	 * Instant change of aim angle.
	 *
	 * A value of 50 is quite fast already, but lags a bit behind the cursor.
	 *
	 * @return proplist the behaviour it self, for further configuration calls.
	 */
	SetAimSpeed = func (int speed)
	{
		if (speed < 0)
		{
			FatalError("Must provide a positive speed value, or 0/nil for default");
		}
		else if (speed == 0)
		{
			speed = nil;
		}
		this.AnimAimSpeed = speed ?? 1800;
		return this;
	},

	/**
	 * Sets a conditional function. The animation is played only
	 * if behaviour->condition(clonk) returns true, or always
	 * if the condition is nil (default).
	 *
	 * @return proplist the behaviour it self, for further configuration calls.
	 */
	SetCondition = func (func condition)
	{
		this.AnimAimCondition = condition; // Play animation if this is nil (play always), or if the evaluated function condition(clonk) is true.
		return this;
	},

	/**
	 * Defines an animation for the behaviour.
	 *
	 * @par animation Either string or an array.
	 *                In the array you can define up to three animations for blending:
	 *                1 animation name given: The animation contains the entire range from aiming 0° to 180°
	 *                2 animation names given: Blend between animation[0] = 0° and animation[1] = 180°
	 *                3 animation names given: Blend between animation[0] = 90°, animation[1] = 0° and animation[2] = 180°
	 *                A string is converted to the 1-animation-array for convenience.
	 * @par name (optional) Specify the animation name, e.g. "Fire" if you want to provide a firing animation.
	 *           By default, or if nil is passed, this changes the aim animation.
	 *           Note, that you need to change the aim animation only when you create the behaviour.
	 *           If you feel the need to do this at runtime, then most likely you should create
	 *           a new stance for that.
	 *
	 * @return proplist the behaviour it self, for further configuration calls.
	 */
	DefineAnimation = func (any animation, string name)
	{
		name = name ?? "AnimAim"; // Default to setting the aim animation
		if (GetType(animation) == C4V_String)
		{
			animation = [animation];
		}
		else if (GetType(animation) == C4V_Array)
		{
			// Everything OK, leave as is
		}
		else
		{
			FatalError("Only string or proplist allowed, got %v: %v", GetType(animation), animation);
		}
		this[name] = animation;
		return this;
	},

	/**
	 * Timer callback, called every frame.
	 */
	Timer = func (object clonk, any channel)
	{
		if (GetType(this.AnimAimStatus) == C4V_Int)
		{
			// Do not keep the aim angle up to date
			if (FrameCounter() < this.AnimAimStatus)
			{
				return;
			}
			// Animation can be resumed
			this.AnimAimStatus = nil;
		}
		var play_animation = !this.AnimAimCondition || this->AnimAimCondition(clonk);
		if (this.AnimAimStatus == nil)
		{
			if (play_animation)
			{
				this.AnimAimStatus = PlayAnimLoop(clonk, "AnimAim") != nil;
			}
		}
		else // Animation is playing
		{
			if (play_animation)
			{
				UpdateAnim(clonk);
			}
			else
			{
				StopAnim(clonk);
			}
		}
	},

	OnStanceSet = func (object clonk, any channel, bool force)
	{
	},

	OnStanceReset = func (object clonk, any channel, bool force)
	{
		StopAnim(clonk);
	},

	StopAnim = func (object clonk)
	{
		// Stop the  animation
		clonk->StopAnimation(clonk->GetRootAnimation(this.AnimSlot));
		this.AnimAimStatus = nil;
		// Allow Clonk to turn freely again.
		clonk->SetTurnForced(-1);
	},

	PlayAnimLoop = func (object clonk, string name)
	{
		var animation_index = nil;
		var animations = this[name];
		var slot = this.AnimSlot;

		// Stop the previous animation, if there is any
		StopAnim(clonk);

		if (animations == nil || GetLength(animations) == 0)
		{
			// TODO
		}
		else
		{
			var anim0 = animations[0];
			var anim1 = animations[1];

			// Do we just have one animation? Then just play it
			if (anim1 == nil)
			{
				animation_index = clonk->PlayAnimation(anim0, slot, Anim_Const(clonk->GetAnimationLength(anim0) / 2), Anim_Const(1000));
			}
			else
			{
				var time = 10; // Provide a basic time. Was aim_set["AimTime"] originally
				animation_index = clonk->PlayAnimation(anim0, slot, Anim_Linear(0, 0, clonk->GetAnimationLength(anim0), time, ANIM_Loop), Anim_Const(1000));
				animation_index = clonk->PlayAnimation(anim1, slot, Anim_Linear(0, 0, clonk->GetAnimationLength(anim1), time, ANIM_Loop), Anim_Const(1000), animation_index);
				animation_index += 1;
				clonk->SetAnimationWeight(animation_index, Anim_Const(500));
			}
		}
		this.AnimAimIndex = animation_index;
		return animation_index;
	},

	PlayAnimSingle = func (object clonk, string name, int aim_angle, int duration)
	{
		var animation_index = nil;
		var animations = this[name];
		var slot = this.AnimSlot;

		// Stop the previous animation, if there is any
		StopAnim(clonk);

		if (animations == nil || GetLength(animations) == 0)
		{
			// TODO
		}
		else
		{
			// Note when to resume the loop animation
			this.AnimAimStatus = FrameCounter() + duration;

			var anim0 = animations[0];
			var anim1 = animations[1];
			var anim2 = animations[2];
			// Do we just have one animation? Then just play it
			if (anim1 == nil)
			{
				animation_index = clonk->PlayAnimation(anim0, slot, Anim_Linear(0, 0, clonk->GetAnimationLength(anim0), duration, ANIM_Remove), Anim_Const(1000));
			}
			// Well two animations blend between them (animation 1 is 0° animation2 for 180°)
			else if (anim2 == nil)
			{
				animation_index = clonk->PlayAnimation(anim0,  slot, Anim_Linear(0, 0, clonk->GetAnimationLength(anim0), duration, ANIM_Remove), Anim_Const(1000));
				animation_index = clonk->PlayAnimation(anim1, slot, Anim_Linear(0, 0, clonk->GetAnimationLength(anim1), duration, ANIM_Remove), Anim_Const(1000), animation_index);
				animation_index += 1;
				SetAnimationWeight(animation_index, Anim_Const(1000 * Abs(aim_angle) / 180));
			}
			// Well then we'll have three to blend (animation 1 is 90°, animation 2 is 0°, animation 2 for 180°)
			else
			{
				if (Abs(aim_angle) < 90)
				{
					animation_index = clonk->PlayAnimation(anim1, slot, Anim_Linear(0, 0, clonk->GetAnimationLength(anim1), duration, ANIM_Remove), Anim_Const(1000));
					animation_index = clonk->PlayAnimation(anim0,  slot, Anim_Linear(0, 0, clonk->GetAnimationLength(anim0), duration, ANIM_Remove), Anim_Const(1000), animation_index);
					animation_index += 1;
					clonk->SetAnimationWeight(animation_index, Anim_Const(1000 * Abs(aim_angle) / 90));
				}
				else
				{
					animation_index = clonk->PlayAnimation(anim0,  slot, Anim_Linear(0, 0, clonk->GetAnimationLength(anim0), duration, ANIM_Remove), Anim_Const(1000));
					animation_index = clonk->PlayAnimation(anim2, slot, Anim_Linear(0, 0, clonk->GetAnimationLength(anim2), duration, ANIM_Remove), Anim_Const(1000), animation_index);
					animation_index += 1;
					clonk->SetAnimationWeight(animation_index, Anim_Const(1000 *(Abs(aim_angle) - 90) / 90));
				}
			}
		}
		return animation_index;
	},

	UpdateAnim = func (object clonk)
	{
		var speed = this.AnimAimSpeed ?? 1800;
		var aim_animation_index = this.AnimAimIndex;
		if (aim_animation_index == nil)
		{
			return false; // if no animation yet
		}

		// Determine position to aim at
		var aim_angle = clonk->~GetAimAnimationAngle();
		if (aim_angle == nil)
		{
			aim_angle = 900 * clonk->~GetCalcDir();
		}
		aim_angle = Normalize(aim_angle, -1800, 10);
		// Update direction
		if (aim_angle < 0)
		{
			clonk->~SetTurnForced(DIR_Left);
		}
		else if (aim_angle > 0)
		{
			clonk->~SetTurnForced(DIR_Right);
		}

		// Aim according to  animation type:
		var animations = this["AnimAim"]; // TODO: Update fire animations, too?
		var use_anim_position = GetLength(animations) == 1;

		var length, position;
		if (use_anim_position)
		{
			length = clonk->GetAnimationLength(animations[0]);
			position = clonk->GetAnimationPosition(aim_animation_index);
		}
		else
		{
			length = 1000;
			position = clonk->GetAnimationWeight(aim_animation_index);
		}
		var angle = position * 1800 / length;
		var delta_angle = BoundBy(Abs(aim_angle) - angle, -speed, speed);
		var blend = Anim_Const((angle + delta_angle) * length / 1800);
		if (use_anim_position)
		{
			clonk->~SetAnimationPosition(aim_animation_index, blend);
		}
		else
		{
			clonk->~SetAnimationWeight(aim_animation_index, blend);
		}
		return true;
	},
};
