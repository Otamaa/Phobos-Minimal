#pragma once

enum class FXExecutionState : int
{
	/// <summary> The System or Emitter simulates and allows spawning. </summary>
	Active,
	/// <summary> The System or Emitter simulates but does not allow any new spawning. </summary>
	Inactive,
	/// <summary> The System or Emitter destroys all Particles it owns, and then moves to the Inactive Execution State. </summary>
	InactiveClear,
	/// <summary> The System or Emitter does not simulate and does not render. </summary>
	Complete
};

enum class FXLifetimeMode : int
{
	DirectSet,
	Random
};

enum class FXMassInitializationMode : int
{
	DirectSet,
	Random
};

enum class FXCoordinateSpace : int
{
	World,
	Local
};