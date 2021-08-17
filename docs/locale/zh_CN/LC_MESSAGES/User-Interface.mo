��    A      $              ,  �   -  �   �  �   ;     �  �   O  _   �     ]     c     w    �     �  #   �  �   �    �	  �  �
  �     �   �     R  c   d     �     �  l   �  r   V  g  �     1     A     S  V   b     �     �  /   �       U   *  �   �  �        �  �   �     �  \   �  [   /  �   �  x     �   �  �     t   �  �   @  \   (     �     �  �   �  x   )  �   �  4   f  �  �  �   Z  E   �  F   <  �   �  B   =  �   �  �     g   �     *      A   w  [   �   �!  }   _"  �   �"  }   k#  �   �#  B   �$     �$     �$     �$    �$     &  '   &  @   =&  �   ~&  �   b'  N   �'  q   <(     �(  -   �(     �(     �(  v   	)  |   �)  D  �)     B+     W+     n+  <   �+     �+     �+  *   �+     
,  P   ,  �   h,  S   �,     O-     b-     �-  ]   �-  E   V.  d   �.  f   /  Q   h/  x   �/  f   30  �   �0  c   u1     �1     �1  �   �1  I   x2  ,   �2  ?   �2  �   /3  �   �3  D   n4  9   �4  �   �4  E   �5  �   �5  �   t6  [   7     s7     �7   ![image](_static/images/harvestercounter-01.gif)   *Harvester Counter in [Fantasy ADVENTURE](https://www.moddb.com/mods/fantasy-adventure)* ![image](_static/images/healthbar.hide-01.png)   *Health bars hidden in [CnC: Final War](https://www.moddb.com/mods/cncfinalwar)* ![image](_static/images/producing-progress-01.gif)   *Producing Progress bars in [Fantasy ADVENTURE](https://www.moddb.com/mods/fantasy-adventure)* ![image](_static/images/tooltips-01.png)   *Extended tooltips used in [CnC: Final War](https://www.moddb.com/mods/cncfinalwar)* ![smartvesters](_static/images/lowpriority-01.gif)   *Harvesters not selected together with battle units in [Rise of the East](https://www.moddb.com/mods/riseoftheeast) mod* An additional counter for your active/total harvesters can be added near the credits indicator. Audio Battle screen UI/UX Bugfixes and miscellanous By setting `HarvesterCounter.ConditionYellow` and `HarvesterCounter.ConditionRed`, the game will warn player by changing the color of counter whenever the active percentage of harvesters less than or equals to them, like HP changing with `ConditionYellow` and `ConditionRed`. Cameo Sorting Custom Missing Cameo (`XXICON.SHP`) Enabled ability to load full-color non-paletted PCX graphics of any bitness. This applies to every single PCX file that is loaded, including the Ares-supported PCX files. Extended tooltips don't use `TXT_MONEY_FORMAT_1` and `TXT_MONEY_FORMAT_2`. Instead you can specify cost, power and time labels (displayed before correspoding values) with the corresponding tags. Characters `$ U+0024`, `⚡ U+26A1` and `⌚ U+231A` are used by default. Fixed `Blowfish.dll`-caused error `***FATAL*** String Manager failed to initialize properly`, which occurred if `Blowfish.dll` could not be registered in the OS, for example, it happened when the player did not have administrator rights. With Phobos, if the game did not find a registered file in the system, it will no longer try to register this file, but will load it bypassing registration. Fixed a bug when switching build queue tabs via QWER didn't make tooltips disappear as they should, resulting in stuck tooltips. Fixed non-IME keyboard input to be working correctly for languages / keyboard layouts that use character ranges other than Basic Latin and Latin-1 Supplement (font support required). Harvester counter Health bar display can now be turned off as needed, hiding both the health bar box and health pips. Hide health bars Hotkey Commands If need localization, just add `TXT_DUMP_OBJECT_INFO` and `TXT_DUMP_OBJECT_INFO_DESC` into your `.csf` file. If need localization, just add `TXT_NEXT_IDLE_HARVESTER` and `TXT_NEXT_IDLE_HARVESTER_DESC` into your `.csf` file. If you use the vanilla font in your mod, you can use {download}`the improved font <_static/files/ImprovedFont-v4.zip>` (v4 and higher) which among everything already includes the mentioned icons. Otherwise you'd need to draw them yourself using [WWFontEditor](http://nyerguds.arsaneus-design.com/project_stuff/2016/WWFontEditor/release/?C=M;O=D), for example. In `RA2MD.ini`: In `rulesmd.ini`: In `uimd.ini`: It's now possible to switch hardcoded sidebar button coords to use GDI sidebar coords. Loading screen Low priority for box selection PCX files can now be used as loadscreen images. Producing Progress SWType's tooltip would display it's name, cost,  and recharge time (when applicable). Same as with harvester counter, you can download {download}`the improved font <_static/files/ImprovedFont-v4.zip>` (v3 and higher) or draw your own icons. Selects and centers the camera on the next TechnoType that is counted via the [harvester counter](#harvester-counter) and is currently idle. Sidebar / Battle UI Sidebar tooltips can now display extended information about the TechnoType/SWType when hovered over it's cameo. In addition the low character limit is lifted when the feature is enabled via the corresponding tag, allowing for 1024 character long tooltips. Specify Sidebar style TechnoType's tooltip would display it's name, cost, power and description (when applicable). The Cameo Priority is checked just before evevything vanilla. Greater `CameoPriority` wins. The counter is displayed with the format of `Label(Active Harvesters)/(Total Harvesters)`. The label is `⛏ U+26CF` by default. The descriptions are designed to be toggleable by users. For now you can only do that externally via client or manually. The loadscreen size can now be different from the default `800x600` one; if the image is bigger than the screen it's centered and cropped. The tooltips can now go over the sidebar bounds to accommodate for longer contents. You can control maximum text width with a new tag (paddings are excluded from the number you specify). This behavior is designed to be toggleable by users. For now you can only do that externally via client or manually. This feature works in conjunction with CnCNet5 spawner DLL which resizes loadscreen window to actual monitor size and places the image in center. If there's no CnCNet5 spawner loaded, the window resolution will be always `800x600`. This page lists all user interface additions, changes, fixes that are implemented in Phobos. Tooltips User Interface Writes currently hovered or last selected object info in log and shows a message. See [this](Miscellanous.md#dump-object-info) for details. You can adjust counter position by `Sidebar.HarvesterCounter.Offset`, negative means left/up, positive means right/down. You can now disable hardcoded black dots that YR engine shows over empty spawn locations, which allows to use prettier and more correctly placed markers that are produced by Map Renderer instead. You can now know your factories' status via sidebar! You can now set lower priority for an ingame object (currently has effect on units mostly), which means it will be excluded from box selection if there's at least one normal priority unit in the box. Otherwise it would be selected as normal. Works with box+type selecting (type select hotkey + drag) and regular box selecting. Box shift-selection adds low-priority units to the group if there are no normal priority units among the appended ones. You can now specify Cameo Priority for any TechnoType/SuperWeaponType. Vanilla sorting rules are [here](https://modenc.renegadeprojects.com/Cameo_Sorting). You can now specify any SHP/PCX file as XXICON.SHP for missing cameo. You can now specify which soundtrack themes would play on win or lose. You can specify custom `gamemd.exe` icon via `-icon` command line argument followed by absolute or relative path to an `*.ico` file (f. ex. `gamemd.exe -icon Resources/clienticon.ico`). You can specify custom loadscreen with Ares tag `File.LoadScreen`. You can specify which TechnoType should be counted as a Harvester. If not set, the techno with `Harvester=yes` or `Enslaves=SOMESLAVE` will be counted. You can use {download}`the improved vanilla font <_static/files/ImprovedFont-v4.zip>` (v4 and higher) which has way more Unicode character coverage than the default one. You need to draw your own assets (`tab0xpp.shp`, x is replaced by 0-3) and put them into `sidec0x.mix`. `[ ]` Dump Object Info `[ ]` Next Idle Harvester Project-Id-Version: Phobos 
Report-Msgid-Bugs-To: 
POT-Creation-Date: 2021-08-16 14:17+0800
PO-Revision-Date: YEAR-MO-DA HO:MI+ZONE
Last-Translator: FULL NAME <EMAIL@ADDRESS>
Language: zh_CN
Language-Team: zh_CN <LL@li.org>
Plural-Forms: nplurals=1; plural=0
MIME-Version: 1.0
Content-Type: text/plain; charset=utf-8
Content-Transfer-Encoding: 8bit
Generated-By: Babel 2.9.1
 ![image](_static/images/harvestercounter-01.gif)  
*矿车指示器在[Fantasy ADVENTURE](https://www.moddb.com/mods/fantasy-adventure)中* ![image](_static/images/healthbar.hide-01.png)  
*隐藏血条在[CnC: Final War](https://www.moddb.com/mods/cncfinalwar)中* ![image](_static/images/producing-progress-01.gif)  
*生产进度条在[Fantasy ADVENTURE](https://www.moddb.com/mods/fantasy-adventure)中* ![image](_static/images/tooltips-01.png)  
*扩展工具提示在[CnC: Final War](https://www.moddb.com/mods/cncfinalwar)中* ![smartvesters](_static/images/lowpriority-01.gif)  
*在战斗单位中矿车不被选中在[Rise of the East](https://www.moddb.com/mods/riseoftheeast)中* 现在可以在金钱指示器旁边追加一个矿车指示器。 声音 战斗界面UI/UX Bug修复及杂项增强 当正在工作的矿车数量低于一定百分比时，矿车指示器将改变它的颜色警示玩家。通过`HarvesterCounter.ConditionYellow`和`HarvesterCounter.ConditionRed`指定百分比，类似血条颜色通过`ConditionYellow`和`ConditionRed`改变。 图标排序 自定义缺省图标（`XXICON.SHP`） 允许载入全彩非索引色的PCX。对Ares的PCX也支持。 扩展工具提示不使用`TXT_MONEY_FORMAT_1`和`TXT_MONEY_FORMAT_2`。取而代之你可以自己指定金钱，电力，时间的标志（在对应值前显示）。默认分别为`$ U+0024`，`⚡ U+26A1`，`⌚ U+231A`。 修复了由`Blowfish.dll`导致的错误`***FATAL*** String Manager failed to initialize properly`。原理见英文原文，不翻译。 修复了当通过QWER切换标签页时, 工具提示不会消失的问题。 修复了非输入法键盘输入以使基础拉丁或拉丁-1以外的语言或键盘布局可以正常工作。 矿车指示器 如果需要，血条可以被关闭显示。 隐藏血条 快捷键指令 如果需要本地化，只需要在`.csf`文件中增加`TXT_DUMP_OBJECT_INFO`和`TXT_DUMP_OBJECT_INFO_DESC`即可。 如果需要本地化，只需要在`.csf`文件中增加`TXT_NEXT_IDLE_HARVESTER`和`TXT_NEXT_IDLE_HARVESTER_DESC`即可。 如果你的mod使用原版字库，可以使用已经包含上文提到过的标志的{download}`增强字库 <_static/files/ImprovedFont-v4.zip>`（v4或更高版本）。否则你需要自行编辑字库，比如使用[WWFontEditor](http://nyerguds.arsaneus-design.com/project_stuff/2016/WWFontEditor/release/?C=M;O=D)。 在`RA2MD.ini`中： 在`rulesmd.ini`中： 在`uimd.ini`中： 现在可以切换侧边栏是否使用GDI侧边栏坐标。 载入屏幕 框选低优先级 载入图像现在可以使用PCX格式。 生产进度 拓展后`SWType`的工具提示将显示名称，所需资金，充能时间。 类似矿车指示器，你可以使用{download}`增强字库 <_static/files/ImprovedFont-v4.zip>`（v3或更高版本）或自己绘制标志。 选择并居中下一个被[矿车指示器]计数且处于空闲状态的单位。 侧边栏/战斗UI 鼠标悬浮在图标上时显示的工具提示现在可以更加详细了。同时，最大字数限制也提高到了1024。 指定侧边栏风格 拓展后`TechnoType`的工具提示将显示名称，所需资金，所需电力，描述。 图标优先级是最优先检查的。大的优先级排在前面。 该指示器以`标志 激活矿车数/总矿车数`的格式显示，标志默认为`⛏ U+26CF`。 描述信息可以由用户决定是否开启，但现在只能内置到客户端内或手动设置。 载入图的大小现在可以大于默认的`800x600`，一定为居中绘制。 工具提示现在可以越过侧边栏边界以容纳更长的内容。可以使用新标签控制最大文本宽度。 用户可以自由指定是否启用这个逻辑。目前只能人工设置或内置到客户端中。 此功能是与CnCNet5生成器Dll搭配使用的，它将把载入界面固定为当前游戏分辨率大小并把载入图置于中间。如果没有CnCNet5生成器，那么载入分辨率将被锁定为`800x600`。 此页面列出了所有火卫一关于用户界面的添加、更改及修复所实装的功能。 工具提示 用户界面 将当前鼠标悬浮或选中的目标信息写入日志并输出信息。详情请见[此处](Miscellanous.md#dump-object-info)。 可以通过``调整指示器的位置。负数左/上，正数右/下。 现在可以关闭YR引擎硬编码的黑点 现在可以通过侧边栏了解到你的工厂的状态了！ 现在可以为游戏内的物体设定为低选择权重。这样的单位将不会在**框选**，**T框选**，**Shift框选**有正常选择权重的单位时被选中。 现在可以为任意单位或超武指定图标优先级。原版排序规则见[这里](https://modenc.renegadeprojects.com/Cameo_Sorting)。 现在可以指定任意SHP/PCX文件为缺省图标（XXICON.SHP） 你可以指定玩家胜利或视频时播放的音轨。 可以通过命令行参数`-icon <路径>`指定`gamemd.exe`的图标，路径可以是绝对路径也可以是相对路径（例如`gamemd.exe -icon Resources/clienticon.ico`）。 自定义载入屏幕可以通过Ares标签`File.LoadScreen`实现。 可以将任意单位指定为被计算的矿车。如果未设定，带有`Harvester=yes`或`Enslaves=SOMESLAVE`的单位将被计算。 你可以{download}`原版增强字库 <_static/files/ImprovedFont-v4.zip>`（v4或更高版本）来覆盖默认字库。此字库拥有更多的Unicode字符。 需要自行在`sidec0x.mix`中添加``tab0xpp.shp`(x为0-3) 作为生产进度的素材。 `[ ]` 输出目标信息 `[ ]` 下一空闲矿车 