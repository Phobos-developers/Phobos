��    \      �              �  H   �  Z   &  P   �  A   �  K     C   `  X   �  %   �  1   #  =   U  	   �  =   �  .   �  -   
	  2   8	  5   k	  R   �	  F   �	  M   ;
  /   �
  M   �
  t     3   |  @   �  r   �  Z   d  8   �  Z   �  l   S  C   �  g     }   l  g   �  _   R  �   �  V   ]  {   �  w   0  ~   �  s   '  U   �  I   �  Y   ;  �   �  �     G   �  i   �     `          �  ,   �     �  /   �  �     <   �  E   �  O   5  F   �  	   �  S   �  p   *  J   �     �  0   �  n        �  o   �  Y   	  �   c  _   D  D   �  L   �  �   6  7     <   F  >   �  U   �  a     7   z  <   �  F   �  P   6  8   �  �   �  %   c      �   &   �   
   �   T   �   V   !  [   v!  w  �!  1   J#  9   |#  >   �#  :   �#  6   0$  2   g$  S   �$     �$     %  5   )%     _%  6   l%  1   �%  *   �%  -    &  2   .&  R   a&  3   �&  Z   �&  $   C'  *   h'  ]   �'  /   �'  W   !(  ]   y(  W   �(  >   /)  X   n)  Y   �)  -   !*  V   O*  f   �*  t   +  W   �+  t   �+  =   O,  d   �,  ?   �,  y   2-  m   �-  N   .  N   i.  z   �.  {   3/  ]   �/  ?   0  o   M0  %   �0     �0  	   �0     1     #1  !   91     [1  &   �1  =   2  B   @2  ;   �2     �2  B   �2  k   	3  ]   u3  	   �3  *   �3  V   4     _4  k   r4  ;   �4  �   5  Z   �5  (   ?6  C   h6  �   �6  )   ]7  #   �7  4   �7  G   �7  P   (8  4   y8  C   �8  3   �8  >   &9  1   e9  T   �9  &   �9     :  !   #:  	   E:  G   O:  J   �:  X   �:   Ability to disable black spawn position dots on map preview (by Belonit) Ability to specify amount of shots for strafing aircraft and burst simulation (by Starkku) Ability to specify applicable building owner for building upgrades (by Kerbiter) Ability to switch to GDI sidebar layout for any side (by Belonit) Allow making technos unable to be issued with movement order (by Uranusian) Anim-to-Unit logic and ability to randomize DestroyAnim (by Otamaa) Basic projectile interception logic (by AutoGavy, ChrisLv_CN, Kerbiter, Erzoid/SukaHati) Burst delays for weapons (by Starkku) Burst-specific FLH's for TechnoTypes (by Starkku) Chance-based critical damage system on warheads (by AutoGavy) Changelog Custom Radiation Types (by AlexB, Otamaa, Belonit, Uranusian) Custom game icon command line arg (by Belonit) Custom per-warhead SplashLists (by Uranusian) Customizable cameo sorting priority (by Uranusian) Customizable disk laser radius (by Belonit, Kerbiter) Customizable harvester active/total counter next to credits counter (by Uranusian) Customizable harvester ore gathering animation (by secsome, Uranusian) Customizable producing progress "bars" like CnC:Remastered did (by Uranusian) Customizeable Missing Cameo file (by Uranusian) Customizeable Teleport/Chrono Locomotor properties per TechnoType (by Otamaa) Deploying mind-controlled TechnoTypes won't make them permanently mind-controlled anymore (unfinished fix by DCoder) Dump Object Info hotkey command (by secsome, FS-21) ElectricBolt arc visuals can now be disabled per-arc (by Otamaa) Extended sidebar tooltips with descriptions, recharge time and power consumption/generation (by Kerbiter, Belonit) Fix to take Burst into account for aircraft weapon shots beyond the first one (by Starkku) Fixed DeathWeapon not detonating properly (by Uranusian) Fixed QWER hotkey tab switching not hiding the displayed tooltip as it should (by Belonit) Fixed `DebrisMaximums` (spawned debris type amounts cannot go beyond specified maximums anymore) (by Otamaa) Fixed an occasional crash when selecting units with a selection box Fixed extended building upgrades logic not properly interact with Ares' BuildLimit check (by Uranusian) Fixed fatal errors when `Blowfish.dll` couldn't be registered in the system properly due to missing admin rights (by Belonit) Fixed laser drawing code to allow for thicker lasers in house color draw mode (by Kerbiter, ChrisLv_CN) Fixed lasers & other effects drawing from wrong offset with weapons that use Burst (by Starkku) Fixed non-IME keyboard input to be working correctly for languages / keyboard layouts that use character ranges other than Basic Latin and Latin-1 Supplement (by Belonit) Fixed occasional crashes introduced by `Speed=0` stationary vehicles code (by Starkku) Fixed the bug that script action `Move to cell` was still using leftover cell calculations from previous games (by secsome) Fixed the bug when `InfiniteMindControl` with `Damage=1` will auto-release the victim to control new one (by Uranusian) Fixed the bug when after a failed placement the building/defence tab hotkeys won't trigger placement mode again (by Uranusian) Fixed the bug when building with `UndeployInto` plays `EVA_NewRallypointEstablished` while undeploying (by secsome) Fixed the bug when cloaked Desolator was unable to fire his deploy weapon (by Otamaa) Fixed the bug when executing the stop command game crashes (by Uranusian) Fixed the bug when trigger action `125 Build At...` didn't play buildup anim (by secsome) Fixed the bug when trigger action `125 Build At...` wasn't actually producing a building when the target cells were occupied (by secsome) Fixed the bug when units are already dead but still in map (for sinking, crashing, dying animation, etc.), they could die again (by Uranusian) Fixed the critical damage logic not functioning properly (by Uranusian) Fixes to `DeployFire` logic (`DeployFireWeapon`, `FireOnce`, stop command now work properly) (by Starkku) For Map Editor (Final Alert 2) From older Phobos versions From vanilla Full-color PCX graphics support (by Belonit) In `FAData.ini`: Initial Strength for TechnoTypes (by Uranusian) Key `rulesmd.ini->[SOMETECHNOTYPE]->Deployed.RememberTarget` is deprecated and can be removed now, the bugfix for `DeployToFire` deployers is now always on. LaserTrails initial implementation (by Kerbiter, ChrisLv_CN) Lifted stupidly small limit for tooltip character amount (by Belonit) Map previews with zero size won't crash the game anymore (by Kerbiter, Belonit) Maximum waypoints amount increased from 702 to 2147483647 (by secsome) Migrating Multiple mind controllers can now release units on overload (by Uranusian, secsome) New ScriptType actions `71 Timed Area Guard`, `72 Load Onto Transports`, `73 Wait until ammo is full` (by FS-21) New warheads now work with Ares' `GenericWarhead` superweapon (by Belonit) New: Optional mind control range limit (by Uranusian) Ore drills now have customizable ore type, range, ore growth stage and amount of cells generated (by Kerbiter) Phobos fixes: Properly rewritten `DeployToFire` fix, tag `Deployed.RememberTarget` is deprecated, now always on (by Kerbiter) Properly rewritten a fix for mind-controlled vehicles deploying into buildings (by FS-21) Radiation now has owner by default, which means that radiation kills will affect score and radiation field will respect `Affects...` entries. You can override that with `rulesmd.ini->[SOMEWEAPONTYPE]->Rad.NoOwner=yes` entry. Re-enable obsolete `JumpjetControls` for TechnoTypes' default Jumpjet properties (by Uranusian) Remove Disguise and Remove Mind Control warhead effects (by secsome) SHP debris hardcoded shadows now respect `Shadow=no` tag value (by Kerbiter) SHP debris hardcoded shadows now respect `Shadow=no` tag value, and due to it being the default value they wouldn't have hardcoded shadows anymore by default. Override this by specifying `Shadow=yes` for SHP debris. Select Next Idle Harvester hotkey command (by Kerbiter) Selection priority filtering for box selection (by Kerbiter) Semantic locomotor aliases for modder convenience (by Belonit) Setting VehicleType `Speed` to 0 now makes game treat them as stationary (by Starkku) Shield logic for TechnoTypes (by Uranusian, secsome, Belonit) with warhead additions (by Starkku) Shroud, reveal and money transact warheads (by Belonit) Sidebar tooltips now can go over sidebar bounds (by Belonit) Spawns can now have the same exp. level as owner techno (by Uranusian) Spawns now can be killed on low power and have limited pursuing range (by FS-21) Support for PCX loading screens of any size (by Belonit) This page lists the history of changes across stable Phobos releases and also all the stuff that requires modders to change something in their mods to accomodate. Tileset 255+ bridge fix (by E1 Elite) Vanilla fixes: Weapon targeting filter (by Uranusian) What's New `AnimList.PickRandom` used to randomize `AnimList` with no side effects (by secsome) `DeployToFire` vehicles won't lose target on deploy anymore (unfinished fix by DCoder) `TurretOffset` now accepts `F,L,H` and `F,L` values instead of just `F` value (by Kerbiter) Project-Id-Version: Phobos 
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
 允许关闭游戏预览时的黑点 (by Belonit) 扫射飞机每轮射击次数与Burst模拟 (by Starkku) 允许将建筑升级建给其它可用所属方 (by Kerbiter) 允许为任意阵营切换GDI侧边栏布局 (by Belonit) 允许使单位无法接受移动指令 (by Uranusian) 动画生成单位与随机DestroyAnim (by Otamaa) 基本的抛射体拦截逻辑 (by AutoGavy, ChrisLv_CN, Kerbiter, Erzoid/SukaHati) 武器Burst间隔 (by Starkku) Burst独立FLH (by Starkku) 基于概率的弹头暴击伤害系统 (by AutoGavy) 更新日志 自定义辐射 (by AlexB, Otamaa, Belonit, Uranusian) 自定义游戏图标命令行参数 (by Belonit) 自定义弹头水花动画 (by Uranusian) 自定义图标排序优先级 (by Uranusian) 自定义飞碟激光范围 (by Belonit, Kerbiter) 资金计数器旁边的可自定义的矿车活动/总共计数器 (by Uranusian) 自定义矿车采矿动画 (by secsome, Uranusian) 像《命令与征服：重制版》一样的自定义生产进度“条” (by Uranusian) 自定义缺省图标 (by Uranusian) 自定义超时空移动参数 (by Otamaa) 部署心灵控制单位不再会使他们永久转变所属方 (未完成的修复by DCoder) 输出目标信息快捷键 (by secsome, FS-21) 特斯拉电弧（ElectricBolt）的视觉特效中单独的弧可以关闭 (by Otamaa) 带有介绍，充能时间，电力生成/消耗的扩展工具提示 (by Kerbiter, Belonit) 修复了扫射攻击的飞机只有第一次攻击时读取Burst的问题 (by Starkku) 修复了DeathWeapon不会正确引爆的问题 (by Uranusian) 修复了当通过QWER切换标签页时, 工具提示不会消失的问题 (by Belonit) 修复了DebrisMaximums（生成的碎片不再能超过对应最大上限） (by Otamaa) 修复了选择单位时偶然的崩溃问题 修复了扩展建筑升级与Ares的BuildLimit检查不兼容的问题 (by Uranusian) 修复了由于无管理员权限导致的`Blowfish.dll`未被注册导致的致命错误 (by Belonit) 修复了激光绘制的代码使其能够在所属方颜色模式下绘制更粗的激光 (by Kerbiter, ChrisLv_CN) 修复了激光和其他效果在Burst武器上绘制偏移错误的问题 (by Starkku) 修复了非输入法键盘输入对于使用基础拉丁/拉丁-1以外的语言/键盘布局的支持 (by Belonit) 修复了`Speed=0`的单位偶尔崩溃的问题 (by Starkku) 修复了脚本动作“移动到坐标”使用先前游戏中的残留计算的问题 (by secsome) 多重心控单位可以在过载前释放单位 (by Uranusian) 修复了当一次摆放失败后，建筑标签/防御标签快捷键（Q/W）无法正确响应的问题 (by Uranusian) 修复了有UndeployInto的建筑在反部署时会播放`EVA_NewRallypointEstablished`的问题 (by secsome) 修复了辐射工兵在隐形时无法使用部署武器的问题 (by Otamaa) 修复了某些情况下按下停止命令时会崩溃的问题 (by Uranusian) 修复了触发动作125“将建筑建造于...” 不能播放建造动画的问题，由新的参数控制 (by secsome) 修复了触发动作125`将建筑建造于...`在目标单元格被占用时将无法将建筑放下的问题 (by secsome) 修复了当单位死亡时仍然可以受到伤害导致多次死亡的问题 (by Uranusian) 修复了暴击伤害逻辑工作失常的问题 (by Uranusian) 修复了DeployFire逻辑（DeployFireWeapon, FireOnce, 及停止指令现在可以正常运作） (by Starkku) 对地图编辑器（Final Alert 2） 由旧版本火卫一 由原版 全彩色PCX支持 (by Belonit) 在`FAData.ini`中： 单位初始血量 (by Uranusian) `rulesmd.ini->[SOMETECHNOTYPE]->Deployed.RememberTarget`已弃用并被移除，对`DeployToFire`的修复现在一直开启。 激光尾迹 (by Kerbiter, ChrisLv_CN) 提升了小的过分的工具提示字符上限 (by Belonit) 0大小的地图预览不再使游戏崩溃 (by Kerbiter, Belonit) 路径点数量上限由702增加到2147483647 (by secsome) 迁移 多重心控可以在过载时释放单位 (by Uranusian, secsome) 新的脚本`71 时效性区域警戒`，`72 向载具中装载`，`73 等待直到装满弹药` (by FS-21) 新的弹头特效可以与Ares的通用弹头（`GenericWarhead`）超武兼容 (by Belonit) 新的： 可选的心灵控制距离 (by Uranusian) 矿柱自定义生成矿石类型，范围，生长阶段和生长格数 (by Kerbiter) 火卫一修复： 重写了`DeployToFire`的修复，标签`Deployed.RememberTarget`已被弃用，永久开启 (by Kerbiter) 重写了被心控的车辆变成建筑的修复 (by FS-21) 辐射现在默认拥有所属方，使得辐射击杀可以被算入得分，且辐射场也会遵循`Affects...`设定。可以通过`rulesmd.ini->[SOMEWEAPONTYPE]->Rad.NoOwner=yes`覆盖此设定。 重新激活废弃的`JumpjetControls`作为单位的Jumpjet参数初始值 (by Uranusian) 反伪装和反心控弹头 (by secsome) SHP碎片硬编码影子现在遵循`Shadow=no`设定 (by Kerbiter) SHP碎片硬编码的影子现在遵循`Shadow=no`，并且由于这就是默认值，所以他们不再拥有硬编码的影子。为SHP指定`Shadow=yes`以覆盖此设定。 选择下一矿车快捷键 (by Kerbiter) 选择优先级筛选 (by Kerbiter) 为modder方便所用的locomotor别称 (by Belonit) 将载具的速度设置为0将认为他们是静止单位 (by Starkku) 护盾逻辑 (by Uranusian, secsome, Belonit) 及对应的新弹头 (by Starkku) 迷雾弹，揭示弹和金币弹弹头 (by Belonit) 侧边栏工具提示现在可以超越侧边栏边界 (by Belonit) 子机与发射者共有经验等级 (by Uranusian) 子机在断电模式下坠毁和有限追击距离 (by FS-21) 支持任意大小的PCX载入屏幕 (by Belonit) 此页面列出了项目的更新日志以及需要modder对mod做的必要改动。 Tileset 255+的桥修复 (by E1 Elite) 原版修复： 武器瞄准筛选 (by Uranusian) 新东西 `AnimList.PickRandom`作为无副作用的随机`AnimList` (by secsome) `DeployToFire`不再会因部署失去目标 (未完成的修复by DCoder) `TurretOffset`现在除了`F`以外还可以接受`F,L,H`和`F,L`作为值 (by Kerbiter) 