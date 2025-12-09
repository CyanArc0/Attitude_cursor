# 姿态鼠标
### 组合键
Ctrl+Alt+M Mode 切换模式。  
Ctrl+Alt+C Console 开关控制台，控制台会显示鼠标位置，惯性，风向量等信息。  
Ctrl+Alt+B reset 将鼠标指针重置到屏幕中央。  
Ctrl+Alt+W Weather 循环切换天气。  
Ctrl+Alt+Q Quit 退出程序。  
Ctrl+Alt+L Lock 禁用组合键，输入密码后解锁。  
按住Alt输入数字 输入密码，默认为0147896325。  
###### 模式  
位置模式 无效果。  
姿态模式 鼠标具有惯性，碰到边缘反弹。  
###### 天气  
Still 无风  
Steady 恒风  
Windy 多风  
Gusty 阵风  
### 参数
~1 等待1s后运行，以此类推。  
M0 M1 设置初始模式，M0为位置模式，M1为姿态模式，默认为M0。  
W0 W1 W2 W3 设置初始天气，W0为Still，以此类推，默认为W0。  
L 启动即禁用组合键。  
示例  
```
Attitude_cursor.exe M1 W3 ~3
```
以 姿态模式 Gusty 启动程序，3s后开始运行。  
