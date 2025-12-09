/*Attitude_cursor 1.3*/
#include<cstdio>
#include<cstdlib>
#include<iostream>
#include<compare>
#include<random>
#include<algorithm>
#include<utility>
#include<bit>
#include<cstdint>
#include<string>
#include<ranges>
#include<thread>
#include<chrono>
#include<memory>
#include<type_traits>
#include<optional>
//#include<variant>
//#include<functional>
#include<Windows.h>
#define Windows
//#include"ctypes/byte_t.hpp"
#include"CAF/argument.hpp"
#include"error.ipp"
#include"point.ipp"

struct loop_iteration
{
	inline static const point screen_resolution = {Windows::GetSystemMetrics(SM_CXSCREEN),Windows::GetSystemMetrics(SM_CYSCREEN)};
	inline static const Windows::DWORD display_refresh_rate = []{
		// 获取当前显示设置
		Windows::DEVMODE devMode = { .dmSize = sizeof(Windows::DEVMODE) };
		Windows::EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &devMode) ? OK : Error("Windows::EnumDisplaySettings failed.");
		return devMode.dmDisplayFrequency;
	}();static_assert(std::is_unsigned_v<Windows::DWORD>&&sizeof(Windows::DWORD)==4&&alignof(Windows::DWORD)==alignof(std::uint32_t));
	struct random
	{
		inline static std::random_device device{};// 随机数引擎的种子源
		inline static std::mt19937 generator{device()};// 以 random_device() 播种的 mersenne_twister_engine
		inline static std::uniform_int_distribution<Windows::LONG> distrib{-16*9,16*9};static_assert(std::is_signed_v<Windows::LONG>&&sizeof(Windows::LONG)==4&&alignof(Windows::LONG)==alignof(std::int32_t));
	};
	struct mutex_guard
	{
		// 定义删除器
		struct handle_deleter
		{
			void operator()(Windows::HANDLE handle)const noexcept
			{
				if (handle != NULL && handle != INVALID_HANDLE_VALUE)
				{
					Windows::CloseHandle(handle) ? OK : Error("Windows::CloseHandle failed.");
				}
			}
		};
		std::unique_ptr<std::remove_pointer_t<Windows::HANDLE>,handle_deleter> mutex_handle;
		mutex_guard(LPCTSTR name)noexcept
		{
			(mutex_handle = std::unique_ptr<std::remove_pointer_t<Windows::HANDLE>,handle_deleter>(Windows::CreateMutex(NULL, FALSE, name))) ? OK : Error("Windows::CreateMutex failed.");
			if(GetLastError() == ERROR_ALREADY_EXISTS)//防多开
			{
				//Quit
				mutex_handle = nullptr;
			}
		}
		operator bool()const noexcept
		{
			return static_cast<bool>(mutex_handle);
		}
	};
	inline static mutex_guard unique_guard{TEXT("Attitude_cursor")};
	enum mode:std::uint8_t{Position,Attitude,/*!*/Manual};
	enum weather:std::uint8_t{Still,Steady,Windy,Gusty};
	struct initialization_option
	{
		std::chrono::seconds wait;
		mode mode;
		weather weather;
		bool lock;
	};
private:
	point current_cursor_position{};
	point previous_cursor_position{};
	mode mode_state = mode::Position;
	union//mode_data
	{
		struct//position_data
		{
		};
		struct//attitude_data
		{
			point cursor_velocity{};
		};
	};
	weather weather_state = weather::Still;
	class weather_base
	{
	public:
		virtual void next() = 0;
		virtual point wind()const = 0;
		//virtual ~weather_base() = default;
	};
	std::unique_ptr<weather_base> weather_pointer;
	/*
	struct basic_weather
	{
		char _[32];
		point weather_pointer->wind()()const
		{
			return reinterpret_cast<weather_base const*>(this)->weather_pointer->wind()();
		}
	};
	basic_weather _data;
	void _f()
	{
		//std::destroy_at<gusty_weather>(&_data);
		weather_data.construct();//std::nothrow
		reinterpret_cast<weather_base*>(&weather_data)->weather_pointer->wind()();
		weather_data.destroy();
	}
	*/
	enum combination_key:char{Ctrl_Alt_M='M',Ctrl_Alt_C='C',Ctrl_Alt_B='B',Ctrl_Alt_W='W',Ctrl_Alt_Q='Q',Ctrl_Alt_L='L'};
	struct
	{
		bool M : 1;//模式
		bool C : 1;//控制台
		bool B : 1;//复位
		bool W : 1;//风
		bool Q : 1;//退出
		bool L : 1;//锁定
		/*Enter password*/
		bool _ : 1;
		bool Alt : 1;
	}
	keys_state{};static_assert(sizeof(keys_state)==1);
	bool log_sign = false;
	bool quit_sign = false;
	struct lock_data
	{
		std::string password;
	};
	std::optional<lock_data> lock_flag;
protected:
	static constexpr std::size_t weather_size = 4;
	class still_weather:public weather_base
	{
		static constexpr weather state = weather::Still;
	public:
		virtual void next()override
		{
		}
		virtual point wind()const override
		{
			return {0,0};
		}
	};
	class steady_weather:public weather_base
	{
		static constexpr weather state = weather::Steady;
		inline static const point wind_velocity{-4,3};//screen_resolution.y*3/1080;
	public:
		virtual void next()override
		{
		}
		virtual point wind()const override
		{
			return wind_velocity;
		}
	};
	class windy_weather:public weather_base
	{
		static constexpr weather state = weather::Windy;
		point wind_velocity{};
	public:
		virtual void next()override
		{
			std::uint32_t random_result = random::generator();
			struct _bs
			{
				bool _x0:1;
				bool _x1:1;
				bool _y0:1;
				bool _y1:1;
				//bool _00:4;
				//bool _01:1;
				//bool _10:1;
				//bool _11:1;
				bool __0:4;//
				bool __1:8;
				bool __2:8;
				bool __3:8;
			};static_assert(sizeof(_bs)==sizeof(random_result));
			auto _rbs = reinterpret_cast<_bs const*>(&random_result);
			//if(_rbs.00&&_rbs._01&&_rbs._10&&_rbs._11)
			{
				point difference = {_rbs->_x0?_rbs->_x1?+1:-1:0,_rbs->_y0?_rbs->_y1?+1:-1:0};
				wind_velocity+=difference;
				if(std::abs(wind_velocity.x)+std::abs(wind_velocity.y)>25)
				{
					wind_velocity-=difference;
				}
			}
		}
		virtual point wind()const override
		{
			return wind_velocity;
		}
	};
	class gusty_weather : public weather_base
	{
		static constexpr weather state = weather::Gusty;
		point wind_velocity{};
		std::size_t timer{display_refresh_rate};
	public:
		virtual void next()override
		{
			if(--timer==0)
			{
				timer = display_refresh_rate;
				wind_velocity = {random::distrib(random::generator)/9,random::distrib(random::generator)/16};
			}
		}
		virtual point wind()const override
		{
			return wind_velocity;
		}
	};
	void initialize_mode_module()noexcept
	{
		switch(mode_state)
		{
		case mode::Position:{
		}break;
		case mode::Attitude:{
			cursor_velocity = {};
		}break;
		};
	}
	void initialize_weather_module()noexcept
	{
		switch(weather_state)
		{
		case weather::Still:{
			weather_pointer.reset(new still_weather);
		}break;
		case weather::Steady:{
			weather_pointer.reset(new steady_weather);
		}break;
		case weather::Windy:{
			weather_pointer.reset(new windy_weather);
		}break;
		case weather::Gusty:{
			weather_pointer.reset(new gusty_weather);
		}break;
		};
	}
	void redirect_console()
	{
		Windows::HANDLE handle = Windows::GetStdHandle(STD_OUTPUT_HANDLE);handle==INVALID_HANDLE_VALUE ? Error("Windows::GetStdHandle failed.") : OK;

		// 重定向标准输入输出到控制台
		std::freopen("CONIN$", "r", stdin) ? OK : Error("std::freopen failed.");
		std::freopen("CONOUT$", "w", stdout) ? OK : Error("std::freopen failed.");
		std::freopen("CONOUT$", "w", stderr) ? OK : Error("std::freopen failed.");
		// 强制 C++ 流使用 C 标准流的缓冲区
		std::ios::sync_with_stdio(true);
		// 清空流状态（避免残留错误）
		std::cin.clear();
		std::cout.clear();
		std::cerr.clear();
		std::clog.clear();

		// 隐藏控制台光标
		Windows::CONSOLE_CURSOR_INFO CursorInfo;
		Windows::GetConsoleCursorInfo(handle, &CursorInfo) ? OK : Error("Windows::GetConsoleCursorInfo failed.");//获取控制台光标信息
		CursorInfo.bVisible = false;//设置光标不可见
		Windows::SetConsoleCursorInfo(handle, &CursorInfo) ? OK : Error("Windows::SetConsoleCursorInfo failed.");//设置控制台光标状态
	}
	void toggle_console()
	{
		if(!Windows::GetConsoleWindow())
		{
			// 分配一个新的控制台窗口
			Windows::AllocConsole() ? OK : Error("Windows::AllocConsole failed.");

			redirect_console();
			
			log_sign = true;
		}
		else
		{
			std::fclose(stdin) ? Error("std::fclose failed.") : OK;
			std::fclose(stdout) ? Error("std::fclose failed.") : OK;
			std::fclose(stderr) ? Error("std::fclose failed.") : OK;
			Windows::FreeConsole() ? OK : Error("Windows::FreeConsole failed.");
			
			log_sign = false;
		}
	}
	void reset_cursor()noexcept
	{
		current_cursor_position = {screen_resolution.x/2,screen_resolution.y/2};
		Windows::SetCursorPos(current_cursor_position.x,current_cursor_position.y) ? OK : Error("Windows::SetCursorPos failed.");
		cursor_velocity = {};
	}
	void switch_mode()noexcept
	{
		mode_state = static_cast<mode>((mode_state+1)%2);
		initialize_mode_module();
	}
	void change_weather()noexcept
	{
		weather_state = static_cast<weather>((weather_state+1)%weather_size);
		initialize_weather_module();
	}
	void quit_loop()noexcept
	{
		quit_sign = true;
	}
	void lock_keyboard()noexcept
	{
		lock_flag.emplace();
	}
	bool check_password()noexcept
	{
		//std::hash<std::string_view>{}("");
		if(lock_flag->password == "0147896325")
		{
			return true;
		}
		else return false;
	}
	template<combination_key key>
	void press_key()
	{
		if constexpr(key == Ctrl_Alt_M) switch_mode();
		if constexpr(key == Ctrl_Alt_C) toggle_console();
		if constexpr(key == Ctrl_Alt_B) reset_cursor();
		if constexpr(key == Ctrl_Alt_W) change_weather();
		if constexpr(key == Ctrl_Alt_Q) quit_loop();
		if constexpr(key == Ctrl_Alt_L) lock_keyboard();
	}
	void poll_keyboard()
	{
		if(lock_flag)
		{
			if(Windows::GetAsyncKeyState(VK_MENU)&0x8000)//Alt
			{
				if(log_sign) std::clog << "Alt";
				if(!keys_state.Alt) keys_state.Alt = true;
				
				if(Windows::GetAsyncKeyState('0')&0x8000||Windows::GetAsyncKeyState(VK_NUMPAD0)&0x8000)
				{
					if(!keys_state._)
					{
						keys_state._ = true;
						lock_flag->password.push_back('0');
					}
				}
				else if(Windows::GetAsyncKeyState('1')&0x8000||Windows::GetAsyncKeyState(VK_NUMPAD1)&0x8000)
				{
					if(!keys_state._)
					{
						keys_state._ = true;
						lock_flag->password.push_back('1');
					}
				}
				else if(Windows::GetAsyncKeyState('2')&0x8000||Windows::GetAsyncKeyState(VK_NUMPAD2)&0x8000)
				{
					if(!keys_state._)
					{
						keys_state._ = true;
						lock_flag->password.push_back('2');
					}
				}
				else if(Windows::GetAsyncKeyState('3')&0x8000||Windows::GetAsyncKeyState(VK_NUMPAD3)&0x8000)
				{
					if(!keys_state._)
					{
						keys_state._ = true;
						lock_flag->password.push_back('3');
					}
				}
				else if(Windows::GetAsyncKeyState('4')&0x8000||Windows::GetAsyncKeyState(VK_NUMPAD4)&0x8000)
				{
					if(!keys_state._)
					{
						keys_state._ = true;
						lock_flag->password.push_back('4');
					}
				}
				else if(Windows::GetAsyncKeyState('5')&0x8000||Windows::GetAsyncKeyState(VK_NUMPAD5)&0x8000)
				{
					if(!keys_state._)
					{
						keys_state._ = true;
						lock_flag->password.push_back('5');
					}
				}
				else if(Windows::GetAsyncKeyState('6')&0x8000||Windows::GetAsyncKeyState(VK_NUMPAD6)&0x8000)
				{
					if(!keys_state._)
					{
						keys_state._ = true;
						lock_flag->password.push_back('6');
					}
				}
				else if(Windows::GetAsyncKeyState('7')&0x8000||Windows::GetAsyncKeyState(VK_NUMPAD7)&0x8000)
				{
					if(!keys_state._)
					{
						keys_state._ = true;
						lock_flag->password.push_back('7');
					}
				}
				else if(Windows::GetAsyncKeyState('8')&0x8000||Windows::GetAsyncKeyState(VK_NUMPAD8)&0x8000)
				{
					if(!keys_state._)
					{
						keys_state._ = true;
						lock_flag->password.push_back('8');
					}
				}
				else if(Windows::GetAsyncKeyState('9')&0x8000||Windows::GetAsyncKeyState(VK_NUMPAD9)&0x8000)
				{
					if(!keys_state._)
					{
						keys_state._ = true;
						lock_flag->password.push_back('9');
					}
				}
				else if(keys_state._) keys_state._ = false;
			}
			else if(keys_state.Alt)
			{
				keys_state.Alt = false;
				if(check_password()) lock_flag.reset();//unlock
				lock_flag->password.clear();
			}
		}
		else//默认行为
		{
			// 检测特定按键
			if(Windows::GetAsyncKeyState(VK_CONTROL)&0x8000 && Windows::GetAsyncKeyState(VK_MENU)&0x8000)//Ctrl+Alt
			{
				if(log_sign) std::clog << "Ctrl+Alt";
				//if(!keys_state.Alt) keys_state.Alt = true;
	
				if (Windows::GetAsyncKeyState('M')&0x8000)
				{
					if(log_sign) std::clog << "+M";
					if(!keys_state.M)
					{
						keys_state.M = true;
						press_key<Ctrl_Alt_M>();
					}
				}
				else if(keys_state.M) keys_state.M = false;
				if (Windows::GetAsyncKeyState('C')&0x8000)
				{
					if(log_sign) std::clog << "+C";
					if(!keys_state.C)
					{
						keys_state.C = true;
						press_key<Ctrl_Alt_C>();
					}
				}
				else if(keys_state.C) keys_state.C = false;
				if (Windows::GetAsyncKeyState('B')&0x8000)
				{
					if(log_sign) std::clog << "+B";
					if(!keys_state.B)
					{
						keys_state.B = true;
						press_key<Ctrl_Alt_B>();
					}
				}
				else if(keys_state.B) keys_state.B = false;
				if (Windows::GetAsyncKeyState('W')&0x8000)
				{
					if(log_sign) std::clog << "+W";
					if(!keys_state.W)
					{
						keys_state.W = true;
						press_key<Ctrl_Alt_W>();
					}
				}
				else if(keys_state.W) keys_state.W = false;
				if (Windows::GetAsyncKeyState('Q')&0x8000)
				{
					if(log_sign) std::clog << "+Q";
					if(!keys_state.Q)
					{
						keys_state.Q = true;
						press_key<Ctrl_Alt_Q>();
					}
				}
				else if(keys_state.Q) keys_state.Q = false;
				if (Windows::GetAsyncKeyState('L')&0x8000)
				{
					if(log_sign) std::clog << "+L";
					if(!keys_state.L)
					{
						keys_state.L = true;
						press_key<Ctrl_Alt_L>();
					}
				}
				else if(keys_state.L) keys_state.L = false;
				/*
				if (Windows::GetAsyncKeyState('`_`')&0x8000)
				{
					if(log_sign) std::clog << "+`_`";
					if(!keys_state.`_`)
					{
						keys_state.`_` = true;
						press_key<Ctrl_Alt_`_`>();
					}
				}
				else if(keys_state.`_`) keys_state.`_` = false;
				*/
			}
			else if(std::bit_cast<bool>(keys_state))keys_state = {};
		}
	}
	void mode_module_work()noexcept
	{
		switch(mode_state)
		{
		case mode::Position:
		{
		}
			break;
		case mode::Attitude:
		{
			cursor_velocity += current_cursor_position - previous_cursor_position;
			current_cursor_position = previous_cursor_position + cursor_velocity;
		}
			break;
		};
	}
	void weather_module_work()noexcept
	{
		weather_pointer->next();
		current_cursor_position += weather_pointer->wind();
	}
	public:
	loop_iteration(initialization_option const& opt={})//Initialization
	{
		if(not unique_guard)
		{
			quit_sign = true;
			return;//Quit
		}

		toggle_console();//close console
		if(AttachConsole(ATTACH_PARENT_PROCESS))
		{
			redirect_console();
		}
		log_sign=Windows::GetConsoleWindow();

		if(log_sign) std::clog << screen_resolution.x<<'*'<<screen_resolution.y;
		if(log_sign) std::clog << ',' << display_refresh_rate << "Hz";

		if(opt.wait.count())
		{
			//if(log_sign) std::clog << " sleep"<<"s...";
			std::this_thread::sleep_for(opt.wait);
		}
		if(opt.mode) mode_state = static_cast<mode>(opt.mode);
		if(opt.weather) weather_state = static_cast<weather>(opt.weather);
		if(opt.lock) lock_flag.emplace();

		initialize_mode_module();
		initialize_weather_module();

		Windows::GetCursorPos(&previous_cursor_position) ? OK : Error("Windows::GetCursorPos failed.");

		if(log_sign) std::clog << std::endl;
	}
	operator bool()const noexcept//Discriminant
	{
		if(quit_sign)[[unlikely]]
			return false;
		else return true;
	}
	void operator++()noexcept//Between
	{
		//using namespace std::literals::chrono_literals;
		//std::this_thread::sleep_for(1000ms/display_refresh_rate);
		Windows::Sleep(1000/display_refresh_rate);
		previous_cursor_position = current_cursor_position;
	}
	void operator()()//Loop
	{
		Windows::GetCursorPos(&current_cursor_position) ? OK : Error("Windows::GetCursorPos failed.");
		if(log_sign) std::clog << "cursor:" << current_cursor_position << ' ';

		mode_module_work();
		if(mode_state)
			if(log_sign) std::clog << "AV:" << cursor_velocity << ' ';
		
		weather_module_work();
		if(weather_state)
			if(log_sign) std::clog << "Wind:" << weather_pointer->wind() << ' ';

		//边界处理
		if(mode_state and weather_state)
		{
			//惯性越界
			if(not(0<=current_cursor_position.x-weather_pointer->wind().x and current_cursor_position.x-weather_pointer->wind().x<screen_resolution.x))
				current_cursor_position.x-=weather_pointer->wind().x;
			//风吹出界
			else if(not(0<=current_cursor_position.x and current_cursor_position.x<screen_resolution.x))
				current_cursor_position.x = std::clamp(current_cursor_position.x,0L,screen_resolution.x-1);
			//y轴，同上
			if(not(0<=current_cursor_position.y-weather_pointer->wind().y and current_cursor_position.y-weather_pointer->wind().y<screen_resolution.y))
				current_cursor_position.y-=weather_pointer->wind().y;
			else if(not(0<=current_cursor_position.y and current_cursor_position.y<screen_resolution.y))
				current_cursor_position.y = std::clamp(current_cursor_position.y,0L,screen_resolution.y-1);

			if(log_sign) std::clog << "GV:" << current_cursor_position-previous_cursor_position << ' ';
		}

		if(mode_state or weather_state)
			Windows::SetCursorPos(current_cursor_position.x,current_cursor_position.y) ? OK : Error("Windows::SetCursorPos failed.");

		poll_keyboard();

		if(log_sign) std::clog << "\t\t\t\t" << '\r' << std::flush;
	}
};
//int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR pCmdLine, int nCmdShow)
int main(int argc,char* argv[])
{
	CAF::argument args(argc,argv);
	using mode = loop_iteration::mode;
	using weather = loop_iteration::weather;
	loop_iteration::initialization_option opt{};
	//for(int i=1;i<argc;++i)argv[i];
	for(auto arg : args|std::views::drop(1))
	{
		if(arg.size()>1&&arg[0]=='~')
			opt.wait = std::chrono::seconds(std::stoi(std::string(arg.substr(1))));

		else if(arg=="M0") opt.mode = mode::Position;
		else if(arg=="M1") opt.mode = mode::Attitude;

		else if(arg=="W0") opt.weather = weather::Still;
		else if(arg=="W1") opt.weather = weather::Steady;
		else if(arg=="W2") opt.weather = weather::Windy;
		else if(arg=="W3") opt.weather = weather::Gusty;

		else if(arg=="L") opt.lock = true;
		//if(arg.size()==1&&arg[0]>='A'&&arg[0]<='Z');
	}
	for(loop_iteration i(opt);bool(i);++i)
	{
		i();
	}
	return 0;
}