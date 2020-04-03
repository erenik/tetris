// Emil Hedemalm
// 2013-06-28

#include "Tetris.h"
#include "Input/InputManager.h"
// Don't include all managers. Ever.

#include "Input/Action.h"

/// Creates default key-bindings for the state.
void Tetris::CreateDefaultBindings()
{
	InputMapping& input = inputMapping;
	//	new Binding
	InputMapping* mapping = &inputMapping;
#define Bind1(a,b) mapping->bindings.Add(new Binding(a,b));
#define Bind1Repeat(a,b) mapping->bindings.Add((new Binding(a,b))->SetActivateOnRepeat(true));
#define Bind2(a,b,c) mapping->bindings.Add(new Binding(a,b,c));
#define Bind3(a,b,c,d) mapping->bindings.Add(new Binding(a,b,c,d));

	// Always nice to be able to pause anywhere..
	Bind1(new Action("Pause/Break"), KEY::PAUSE_BREAK);

	// For toggling mouse input.
	Bind3(Action::FromString("IgnoreMouseInput"), KEY::CTRL, KEY::I, KEY::M);
	Bind3(Action::FromString("List cameras"), KEY::CTRL, KEY::L, KEY::C);

	Bind1(Action::FromString("Reset Camera"), KEY::HOME);
	Bind1Repeat(Action::FromString("Left"), KEY::LEFT);
	Bind1Repeat(Action::FromString("Right"), KEY::RIGHT);
	Bind1Repeat(Action::FromString("Down"), KEY::DOWN);
	Bind1Repeat(Action::FromString("Rotate clockwise"), KEY::CTRL);
	Bind1Repeat(Action::FromString("Rotate counter-clockwise"), KEY::SPACE);

}

/// Creates the user interface for this state
void Tetris::CreateUserInterface(){
	if (ui)
		delete ui;
	ui = new UserInterface();
	ui->Load("gui/Tetris.gui");
}
