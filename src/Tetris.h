/// Emil Hedemalm
/// 2015-01-06
/// Our version of Tetris

#include "AppStates/AppState.h"

class TetrisBrick 
{
public:
	/** Types correspond to the following shapes.
		I,	S,	Z,	L,	J,	O,	T

		#			#	 #
		#   ##	##	#	 #	##	#
		#  ##	 ##	##	##	## ###
		#
	*/
	enum {
		I, S, Z, L, J, O, T, MAX_TYPES
	};
	TetrisBrick();
	virtual ~TetrisBrick();
	/// Returns part locations for this brick in local space. This should then be offset by the actual position to get the correct placement.
	List<Vector2f> GetLocalPartLocations();
	List<Vector2i> GetAbsPartLocations();

	/// 0 - default, 1 - 90 degrees, 2 = 180 degrees, 3 = 270 degrees, 4 = 0
	int rotation;

	List<Entity*> parts;
	int type;
	Vector2f position;
	int millisecondsAlive;
};

class Tetris : public AppState 
{
public:
	Tetris();
	/// Virtual destructor to discard everything appropriately.
	virtual ~Tetris();

	/// Function when entering this state, providing a pointer to the previous StateMan.
	virtual void OnEnter(AppState * previousState);
	/// Main processing function, using provided time since last frame.
	virtual void Process(int timeInMs);
	/// Function when leaving this state, providing a pointer to the next StateMan.
	virtual void OnExit(AppState * nextState);

	/// Callback function that will be triggered via the MessageManager when messages are processed.
	virtual void ProcessMessage(Message * message);

	/// Creates default key-bindings for the state.
	virtual void CreateDefaultBindings();

	void CreateUserInterface();
	
private:

	void SpawnNewBrick();
	void UpdateMasterVolumeAndUI();
	void PushSplashScreen();
	void HideSplashScreen();

	void CreateField();
	void SetupCamera();

	
	void EnsureBrickWithinBoundaries(std::shared_ptr<TetrisBrick> brick);
	void CreateParts(std::shared_ptr<TetrisBrick> forBrick);

	/// Unlinks block parts from the field, used before movements.
	void Unlink(List<Entity*> blockParts);
	/// Checks if the block can be placed in the field properly. Only Y+ may go beyond the field's limits.
	bool CanPlace(std::shared_ptr<TetrisBrick> block);
	/// Places. Calls CanPlace automatically unless alreadyChecked is true.
	bool Place(std::shared_ptr<TetrisBrick> block);

	// Clear the field.
	void ClearField();
	// Set things up.
	void NewGame();
	/// End current game. Display stuff.
	void GameOver();

	void UpdateTimeStepMsBasedOnLevel();
	void LevelUp(int levels = 1);
	void AutoUpPoints();
	void UpScore(int points);

	/// Returns false if it failed due to obstruction.
	bool MoveBrick(std::shared_ptr<TetrisBrick> brick, Vector2f distance, bool applyBonusSinceSpedUp = false);
	/// Returns false if it failed due to obstruction.
	bool RotateBrick(std::shared_ptr<TetrisBrick> brick, int turnsClockwise);

	// Evaluate rows if any was completed.
	void EvaluateRows(bool spedUp);


	int millisecondsPassed;
	Entity *** field;
	
	// All brick-parts on the field!
	List<Entity*> brickParts;

	std::shared_ptr<TetrisBrick> movingBrick;
	List<Entity*> movingBrickParts;

	int rowsCompletedTotal;
	int score, highscore;
	int timeStepMs;
	int level;
	int millisecondsSinceLastAutoLevelUp;
};

