/// Emil Hedemalm
/// 2015-01-06
/// Our version of Tetris

#include "Application/Application.h"

#include "Tetris.h"
#include "Maps/MapManager.h"
#include "Model/ModelManager.h"
#include "TextureManager.h"
#include "Random/Random.h"
#include "Physics/PhysicsManager.h"
#include "Physics/Messages/PhysicsMessage.h"

#include "Audio/Messages/AudioMessage.h"

#include "Graphics/Camera/Camera.h"
#include "Graphics/GraphicsManager.h"
#include "Graphics/Messages/GMCamera.h"
#include "Graphics/Messages/GMSetEntity.h"
#include "Graphics/Messages/GMSet.h"
#include "Graphics/Messages/GMUI.h"

#include "Input/Keys.h"

#include "Message/MessageManager.h"
#include "Message/Message.h"

#include "File/LogFile.h"
#include "StateManager.h"
#include "Graphics/Fonts/TextFont.h"

Vector2i fieldSize(10,20);

Random tetrisRand;
Random typeRand;

bool playing = false;

TetrisBrick::TetrisBrick()
{
	millisecondsAlive = 0;
	rotation = 0;
}

TetrisBrick::~TetrisBrick()
{
//	std::cout<<"\nBrick destructor.";
}	

List<Vector2f> TetrisBrick::GetLocalPartLocations()
{
	List<Vector2f> partLocs;
	switch(type)
	{
		case TetrisBrick::I:partLocs.Add(Vector2f(0,2), Vector2f(0, 1), Vector2f(0,0), Vector2f(0,-1)); break;
		case TetrisBrick::J: partLocs.Add(Vector2f(0,1), Vector2f(0, 0), Vector2f(0,-1), Vector2f(-1,-1)); break;
		case TetrisBrick::L: partLocs.Add(Vector2f(0,1), Vector2f(0, 0), Vector2f(0,-1), Vector2f(1,-1)); break;
		case TetrisBrick::O: partLocs.Add(Vector2f(0,1), Vector2f(1, 1), Vector2f(0,0), Vector2f(1,0)); break;
		case TetrisBrick::S: partLocs.Add(Vector2f(0,0), Vector2f(1, 0), Vector2f(0,-1), Vector2f(-1,-1)); break;
		case TetrisBrick::Z: partLocs.Add(Vector2f(0,0), Vector2f(-1, 0), Vector2f(0,-1), Vector2f(1,-1)); break;
		case TetrisBrick::T: partLocs.Add(Vector2f(0,0), Vector2f(-1, 0), Vector2f(+1,0), Vector2f(0,+1)); break;
		default: partLocs.Add(Vector2f());
	}
	return partLocs;
}

List<Vector2i> TetrisBrick::GetAbsPartLocations()
{
	List<Vector2f> locLocs = GetLocalPartLocations();
	List<Vector2i> absLocs;
	for (int i = 0; i < locLocs.Size(); ++i)
	{
		// Rotate as needed?
		Vector2f localLocation = locLocs[i];
		Vector2f rotated = Matrix2f::InitRotationMatrixZ(rotation * (PI * 0.5)).Product(localLocation);
		Vector2f absLoc = rotated + position;
		Vector2i absLoci = absLoc;
		absLocs.Add(absLoci);
	}
	return absLocs;
}


Tetris::Tetris()
: AppState()
{
	millisecondsPassed = 0;
	movingBrick = NULL;
	field = NULL;
	highscore = 0;
	score = 0;
	playing = false;
}

/// Virtual destructor to discard everything appropriately.
Tetris::~Tetris()
{
}

/// Function when entering this state, providing a pointer to the previous StateMan.
void Tetris::OnEnter(AppState * previousState)
{
	TextFont::defaultFontSource = "img/fonts/font3.png";

	if (!ui)
		CreateUserInterface();
	QueueGraphics(new GMSetUI((UserInterface*)ui));
	PushSplashScreen();

	Sleep(1000);
}

void Tetris::PushSplashScreen() {
	QueueGraphics(GMPushUI::ToUI("gui/SplashScreen.gui", ui)); // Push splash screen on top
}
void Tetris::HideSplashScreen() {
	QueueGraphics(new GMPopUI("gui/SplashScreen.gui"));
}


void Tetris::NewGame() {
	HideSplashScreen();
	playing = true;
	// Create the field?
	ClearField();
	CreateField();
	SetupCamera();
	level = 1;
	UpdateTimeStepMsBasedOnLevel();
	score = 0;
	rowsCompletedTotal = 0;
	millisecondsSinceLastAutoLevelUp = 0;
	QueueGraphics(new GMSetUIi("Level", GMUI::INTEGER_INPUT, level));
	QueueGraphics(new GMSetUIi("Score", GMUI::INTEGER_INPUT, score));
	QueueAudio(new AMPlayBGM("audio/2019-11-23_Wait_and_see.ogg", 0.7f));
}

void Tetris::UpdateTimeStepMsBasedOnLevel() {
	timeStepMs = 500 - level * 5; // Starting off at 500ms, decreases to 450ms at level 10, 400ms at level 20, 350ms at level 30, and so on.
}

void Tetris::LevelUp(int levels) {
	level += levels;
	UpdateTimeStepMsBasedOnLevel();
	QueueGraphics(new GMSetUIi("Level", GMUI::INTEGER_INPUT, level));
	if (level >= 30) // Play next BGM 
	{
		QueueAudio(new AMPlayBGM("audio/2020-02-01_Second.ogg", 0.8f));
	}
}

void Tetris::AutoUpPoints() {
	// Gain level points every tick.
	UpScore(level);
}

void Tetris::UpScore(int points) {
	score += points;
	QueueGraphics(new GMSetUIi("Score", GMUI::INTEGER_INPUT, score));
}


void Tetris::SpawnNewBrick() {
	// Spawn a new brick and its segmented parts.
	LogMain("Spawning block", INFO);
	std::cout << "\nSpawwwnnn";
	Vector3f position;
	// Create brick.
	movingBrick = std::make_unique<TetrisBrick>();
	movingBrick->type = typeRand.Randi(TetrisBrick::MAX_TYPES);
	// Place it somewhere in X, 4 units above?
	movingBrick->position.y = fieldSize.y;
	movingBrick->position.x = tetrisRand.Randi(fieldSize.x);
	// Ensure it is within the accepted boundaries.
	EnsureBrickWithinBoundaries(movingBrick);
	CreateParts(movingBrick);
	// Place it.
	bool okToPlace = CanPlace(movingBrick);
	if (okToPlace)
	{
		Place(movingBrick);
	}
	else
	{
		// Game over.
		GameOver();
	}
}

/// Main processing function, using provided time since last frame.
void Tetris::Process(int timeInMs)
{
	if (!playing)
		return;

	millisecondsPassed += timeInMs;
	millisecondsSinceLastAutoLevelUp += timeInMs;
	// Auto-up level every 10 seconds.
	if (millisecondsSinceLastAutoLevelUp > 10000) {
		LevelUp();
		millisecondsSinceLastAutoLevelUp = 0;
	}

	// Check time-step
	if (movingBrick && millisecondsPassed > timeStepMs)
	{
		AutoUpPoints();

		// Unable to move more?! o.o
		if (!MoveBrick(movingBrick, Vector2f(0,-1)))
		{
			// Check if it's sticking out on top, because if it is, it's game over.
			List<Vector2i> positions = movingBrick->GetAbsPartLocations();
			for (int i = 0; i < positions.Size(); ++i)
			{
				if (positions[i].y > fieldSize.y)
				{
					GameOver();
					return;
				}
			}
			// Evaluate rows if any was completed.
			EvaluateRows(false);
			movingBrick = nullptr; // Request new brick by removing grip to old one.
		}
		millisecondsPassed -= timeStepMs;
	}

	if (!movingBrick)
		SpawnNewBrick();
}

/// Function when leaving this state, providing a pointer to the next StateMan.
void Tetris::OnExit(AppState * nextState)
{
	movingBrick = nullptr;
	ClearField();
}

bool muted = false;

void Tetris::UpdateMasterVolumeAndUI() {
	LogMain("Audio set to muted: " + muted, INFO);
	if (muted) {
		QueueAudio(new AMSet(AT_MASTER_VOLUME, 0.0f));
		QueueGraphics(new GMSetUIs("ToggleAudio", GMUI::TEXT, "Toggle Sound On"));
	}
	else {
		QueueAudio(new AMSet(AT_MASTER_VOLUME, 1.0f));
		QueueGraphics(new GMSetUIs("ToggleAudio", GMUI::TEXT, "Toggle Sound Off"));
	}
}

/// Callback function that will be triggered via the MessageManager when messages are processed.
void Tetris::ProcessMessage(Message * message)
{
	String msg = message->msg;

	if (msg == "OnReloadUI") {
		if (!playing)
			PushSplashScreen();
	}
	else if (msg == "NewGame" || msg == "StartGame")
		NewGame();
	else if (msg == "StartMuted") {
		NewGame();
		muted = true;
		UpdateMasterVolumeAndUI();
	}
	else if (msg == "ToggleAudio") {
		muted = !muted;
		UpdateMasterVolumeAndUI();
	}
	else if (msg == "Reset Camera")
		SetupCamera();
	else if (msg == "Left")
	{
		MoveBrick(movingBrick, Vector2f(-1,0));
	}
	else if (msg == "Right")
	{
		MoveBrick(movingBrick, Vector2f(1,0));
	}
	else if (msg == "Down") {
		MoveBrick(movingBrick, Vector2f(0, -1), true);
	}
	else if (msg == "Rotate clockwise")
	{
		RotateBrick(movingBrick, 1);
	}
	else if (msg == "Rotate counter-clockwise")
	{
		RotateBrick(movingBrick, -1);	
	}
	else if (msg == "Exit") {
		Application::live = false;
	}
	else if (msg.Length() > 0)
		LogMain("Received message: " + msg, INFO);

}


/*
/// Creates the user interface for this state. Is called automatically when re-building the UI with the CTRL+R+U command.
void Tetris::CreateUserInterface()
{ 

}
*/

#include "Application/Application.h"

void SetApplicationDefaults()
{
	Application::name = "Tetris";
}

#include "Physics/Integrator.h"

void RegisterStates()
{
	Tetris * tetris = new Tetris();
	StateMan.RegisterState(tetris);
	StateMan.QueueState(tetris);

	PhysicsMan.QueueMessage(new PMSet(new NoIntegrator()));
	QueueGraphics(new GMSet(GT_RENDER_GRID, false)); // Disable debug triangle and grid.
}

void Tetris::CreateField()
{
	field = new Entity ** [fieldSize.x];
	for (int i = 0; i < fieldSize.x; ++i)
	{
		field[i] = new Entity * [fieldSize.y];
		for (int j = 0; j < fieldSize.y; ++j)
		{
			field[i][j] = NULL;
		}
	}

	Texture * tex = NULL;
	int color = 0xFFFF00FF;
//	tex = TexMan.GetTextureByHex32(color); Works, yellow.
	tex = TexMan.GetTextureBySource("img/background.png");
	Entity* background = MapMan.CreateEntity("Background", ModelMan.GetModel("Sprite"), tex, Vector3f(fieldSize.x / 2, fieldSize.y / 2, -2));
	QueuePhysics(new PMSetEntity(background, PT_SET_SCALE, Vector3f(40.0f, 40.0f, 1)));

	// Left Wall
	int wallColor = 0xAAAAAAAA;
	Texture * wallTex = TexMan.GetTextureByHex32(wallColor);
	Entity* topWall = MapMan.CreateEntity("Topwall", ModelMan.GetModel("Sprite"), wallTex, Vector3f(fieldSize.x / 2 - 0.5f, fieldSize.y, 1.0f));
	topWall->scale = Vector3f(fieldSize.x+2, 1.0f, 1.0f);
	Entity* bottomWall = MapMan.CreateEntity("Bottomwall", ModelMan.GetModel("Sprite"), wallTex, Vector3f(fieldSize.x / 2 - 1, -1.0f, 1.0f));
	bottomWall->scale = Vector3f(fieldSize.x+1, 1.0f, 1.0f);
	Entity* rightWall = MapMan.CreateEntity("Rightwall", ModelMan.GetModel("Sprite"), wallTex, Vector3f(fieldSize.x, fieldSize.y / 2 - 0.5f, 1.0f));
	rightWall->scale = Vector3f(1.0f, fieldSize.y+2, 1.0f);
	Entity* leftWall = MapMan.CreateEntity("Leftwall", ModelMan.GetModel("Sprite"), wallTex, Vector3f(-1.0f, fieldSize.y / 2 - 0.5f, 1.0f));
	leftWall->scale = Vector3f(1.0f, fieldSize.y+2, 1.0f);
	// Bottom
	// Top
}

void Tetris::SetupCamera()
{
	// Grab current camera, or just create own..?
	Camera * tetrisCamera = CameraMan.NewCamera("Tetris Camera", true);
	tetrisCamera->smoothness = 0.0f;
	tetrisCamera->rotation = Vector3f();
	tetrisCamera->position = Vector3f(fieldSize.x * 0.5,fieldSize.y * 0.5,20);
//	tetrisCamera->position = Vector3f();
	tetrisCamera->projectionType = Camera::ORTHOGONAL;
	tetrisCamera->zoom = 15.f;

	GraphicsMan.QueueMessage(new GMSetCamera(tetrisCamera));
}

void Tetris::EnsureBrickWithinBoundaries(std::shared_ptr<TetrisBrick> brick)
{
	List<Vector2i> abs = movingBrick->GetAbsPartLocations();
	for (int i = 0; i < abs.Size(); ++i)
	{
		if (abs[i].x < 0)
		{
			movingBrick->position.x += 1;
			return EnsureBrickWithinBoundaries(brick);
		}
		if (abs[i].x >= fieldSize.x)
		{
			movingBrick->position.x -= 1;
			return EnsureBrickWithinBoundaries(brick);
		}
	}
}

void Tetris::CreateParts(std::shared_ptr<TetrisBrick> brick)
{
	Texture * tex = NULL;
	int color = 0xFF0000FF;
	switch(brick->type)
	{
		case TetrisBrick::I: color = 0x00FF00FF; break;
		case TetrisBrick::J: color = 0x2255FFFF; break;
		case TetrisBrick::L: color = 0xFF0000FF; break;
		case TetrisBrick::O: color = 0xFFFF00FF; break;
		case TetrisBrick::S: color = 0xAAAAAAFF; break;
		case TetrisBrick::T: color = 0xFF00FFFF; break;
		case TetrisBrick::Z: color = 0x00FFAAFF; break;
	}
	tex = TexMan.GetTextureByHex32(color);

	List<Vector2f> partLocations = brick->GetLocalPartLocations();

	for (int i = 0; i < partLocations.Size(); ++i)
	{
		Vector2f partLoc = partLocations[i];
		// Create its parts.
		Entity * brickPart = MapMan.CreateEntity("Brick-part", ModelMan.GetModel("Sprite"), tex, partLoc + brick->position);
		brickPart->physics = new PhysicsProperty();
		brickPart->physics->collisionsEnabled = false;
		brickParts.Add(brickPart);
		// Add part to brick.
		brick->parts.Add(brickPart);
	}
	// Scale 'em.
	PhysicsMan.QueueMessage(new PMSetEntity(brick->parts, PT_SET_SCALE, 0.9f));
}


/// Unlinks block parts from the field, used before movements.
void Tetris::Unlink(List<Entity*> blockParts)
{
	int blocksNulled = 0;
	for (int i = 0; i < fieldSize.x; ++i)
	{
		for (int j = 0; j < fieldSize.y; ++j)
		{
			if (blockParts.Exists(field[i][j]))
			{
//				std::cout<<"\nBlock removed at x"<<i<<" y"<<j;
				field[i][j] = NULL;
				++blocksNulled;
			}
		}
	}
	int partNulledInField = blocksNulled;
//	std::cout<<"\nBlock parts removed from "<<blocksNulled<<" locations on the field";
	// Take into consideration blocks beyond the upper border.
	for (int i = 0; i < blockParts.Size(); ++i)
	{
		if (blockParts[i]->localPosition.y >= fieldSize.y)
			++blocksNulled;
	}
//	assert(blocksNulled >= 4);
}

/// Checks if the block can be placed in the field properly. Only Y+ may go beyond the field's limits.
bool Tetris::CanPlace(std::shared_ptr<TetrisBrick> block)
{
	if (field == nullptr)
		return false;

	List<Vector2i> positions = block->GetAbsPartLocations();
	for (int i = 0; i < positions.Size(); ++i)
	{
		Vector2i pos = positions[i];
		if (pos.x < 0 || pos.y < 0 ||
			pos.x >= fieldSize.x)
			return false;
		/// Allow blocks above the Y+ border.
		if (pos.y >= fieldSize.y)
			continue;
		/// check vacancy in the field.
		if (field[pos.x][pos.y] != NULL)
			return false;
	}
	return true;
}

/// Places. Calls CanPlace automatically unless alreadyChecked is true.
bool Tetris::Place(std::shared_ptr<TetrisBrick> block)
{	
	List<Vector2i> absLocs = block->GetAbsPartLocations();
	for (int i = 0; i < absLocs .Size(); ++i)
	{
		Vector2i pos = absLocs[i];
		assert(pos.x >= 0 && pos.y >= 0);
		assert(pos.x < fieldSize.x);
		Entity * part = block->parts[i];
		/// Allow blocks above the Y+ border.
		if (pos.y >= fieldSize.y)
		{
			// Make invisible for the time being?
			GraphicsMan.QueueMessage(new GMSetEntityb(part, GT_VISIBILITY, false));
			continue;
		}
		// Make invisible for the time being?
		GraphicsMan.QueueMessage(new GMSetEntityb(part, GT_VISIBILITY, true));

		/// Set field to point to it.
		assert(field[pos.x][pos.y] == NULL);
		field[pos.x][pos.y] = part;

		/// Set position of rendered entity.
		Vector3f position3f = pos;
		PhysicsMan.QueueMessage(new PMSetEntity(part, PT_SET_POSITION, position3f));
	}
	return true;
}

void Tetris::ClearField() {
	// Delete all entities.
	MapMan.DeleteAllEntities();
	if (field != NULL) {
		for (int i = 0; i < fieldSize.x; ++i)
		{
			delete[] field[i];
		}
		delete[] field;
		field = NULL;
	}
	movingBrick = nullptr; 	// Delete the moving brick (unique ptr)
}

/// End current game. Display stuff.
void Tetris::GameOver()
{
	LogMain("Game over!", INFO);
	QueueAudio(new AMPlayBGM("audio/2019-09-05_Reboot.ogg", 0.6f));

	if (score > highscore)
		highscore = score;
	QueueGraphics(new GMSetUIi("HighScore", GMUI::INTEGER_INPUT, highscore));

	playing = false;
	PushSplashScreen();
}

/// Returns false if it failed due to obstruction.
bool Tetris::MoveBrick(std::shared_ptr<TetrisBrick> brick, Vector2f distance, bool spedUp)
{
	if (brick == nullptr || playing == false)
		return true;
	Unlink(brick->parts);
	// Make a copy
	std::shared_ptr<TetrisBrick> hypoMove = std::make_shared<TetrisBrick>(*brick.get()); 
	// Check location of brick parts if we move it down one step.
	hypoMove->position += distance;
	bool ok = CanPlace(hypoMove);
	if (ok)
	{
		Place(hypoMove);
		brick->position = hypoMove->position;
		QueueAudio(new AMPlaySFX("audio/2020-04-23_Tetris_Move_SFX.wav", 0.5f));
		if (spedUp) 			// Bonus points for speeding
			UpScore(level * 3);
		return true;
	}	
	Place(brick);
	if (spedUp)
		EvaluateRows(spedUp);
	return false;
}


/// Returns false if it failed due to obstruction.
bool Tetris::RotateBrick(std::shared_ptr<TetrisBrick> brick, int turnsClockwise)
{
	if (brick == nullptr)
		return false;
	Unlink(brick->parts);
	// Move brick?
	std::shared_ptr<TetrisBrick> hypoMove = std::make_shared<TetrisBrick>(*brick.get());
	// Check location of brick parts if we move it down one step.
	hypoMove->rotation += turnsClockwise;
	// Round it out?
	if (hypoMove->rotation == 4)
		hypoMove->rotation = 0;
	// Force it to be positive, between 0 and 3 (inclusive) preferably.
	if (hypoMove->rotation < 0)
		hypoMove->rotation += 4;
	// Type-specific rotation.
	switch(brick->type)
	{
		case TetrisBrick::O: hypoMove->rotation = 0; break;
		case TetrisBrick::I:
		case TetrisBrick::S:
		case TetrisBrick::Z:
			// Clamp rotations to 0 and 1.
			hypoMove->rotation = hypoMove->rotation % 2;
			break;
	}

	bool ok = CanPlace(hypoMove);
	if (ok)
	{
		Place(hypoMove);
		brick->rotation = hypoMove->rotation;
		return true;
	}	
	Place(brick);
	return false;
}

void Tetris::EvaluateRows(bool spedUp)
{
	List<int> rowsComplete;
	for (int y = 0; y < fieldSize.y; ++y)
	{
		bool rowComplete = true;
		for (int x = 0; x < fieldSize.x; ++x)
		{
			if (field[x][y])
			{
			
			}
			// Nothing there.
			else 
			{
				// Break inner loop?
				rowComplete = false;
			}
		}
		if (rowComplete)
		{
			rowsComplete.Add(y);
		}
	}
	if (rowsComplete.Size() == 0)
		return;
	
	// Row complete, remove grip to moving brick to prevent any bugs since it shouldn't move any more.
	movingBrick = nullptr;

	// Kill rows! o.o
	for (int i = 0; i < rowsComplete.Size(); ++i)
	{
		int row = rowsComplete[i];
		for (int x = 0; x < fieldSize.x; ++x)
		{
			MapMan.DeleteEntity(field[x][row]);
			field[x][row] = NULL;
		}
	}

	QueueAudio(new AMPlaySFX("audio/2020-04-23_Tetris_RowClear_SFX.wav", 0.5f + rowsComplete.Size() * 0.1f));// Volume from 0.6 to 0.9 (1-4 rows)


	// Give score! o.o
	UpScore((1000 + 100 * level) * pow(2.0f, (float)(rowsComplete.Size() - 1)));
	LevelUp(rowsComplete.Size()); // Increase levels based on how many rows were completed.
	
	LogMain("Score "+score, INFO);

	rowsCompletedTotal += rowsComplete.Size();

	// Animation...! o.o ?
	if (spedUp)
		Sleep(50);
	else 
		Sleep(500);

	// Move down shit! o.o
	// Begin with upper most row which was completed.
	for (int i = rowsComplete.Size() - 1; i >= 0; --i)
	{
		// And move all content above down one step.
		int clearedRow = rowsComplete[i];
		// 
		for (int y = clearedRow + 1; y < fieldSize.y; ++y)
		{
			for (int x = 0; x < fieldSize.x; ++x)
			{
				if (field[x][y])
				{
					// Move it down internally.
					field[x][y - 1] = field[x][y];
					Entity * entity = field[x][y];
					field[x][y] = NULL;
					// Move it down graphically.
					PhysicsMan.QueueMessage(new PMSetEntity(entity, PT_SET_POSITION, Vector3f(x,y-1,0))); 
				}
			}
		}
	}
}
