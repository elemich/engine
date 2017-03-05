#include "datatypes.h"


TouchInput InputManager::touchInput;
MouseInput InputManager::mouseInput;
KeyboardInput InputManager::keyboardInput;
InputInterface InputManager::voiceInput;
InputInterface InputManager::joystickInput;


MouseInput::MouseInput()
{
	memset(mouse_states,MOUSE_STATE_INACTIVE,BUTTON_MAX*sizeof(int));
}

bool  MouseInput::IsClick(int button){return mouse_states[button]==MOUSE_STATE_CLICKED;}


void MouseInput::Update()
{
		
}