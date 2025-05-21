#pragma once
namespace Bamboo {

	class Vector2;
	class Input
	{
		public:
		Input();
		~Input();
		
		// 按键按下
		bool IsKeyDown(int key);
		// 按键释放
		bool IsKeyReleased(int key);
		// 按键是否按下
		bool IsKeyPressed(int key);
		
	};
}; // namespace Bamboo   