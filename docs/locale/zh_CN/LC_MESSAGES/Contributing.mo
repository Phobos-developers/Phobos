��          \               �   �   �      $     1     M  |   _  �   �  4   �  q  �  Q   q     �     �     �  Z   �  �   H  .   '   Assuming you've successfully cloned and built the project before getting here, you should end up with the following project structure: Contributing Guidelines for contributors Project structure This page describes ways to help or contribute to Phobos and lists the contributing guidelines that are used in the project. `Commands/` - source code for new hotkey commands. Every command is a new class that inherits from `PhobosCommandClass` (defined in `Commands.h`) and is defined in a separate file with a few methods and then registered in `Commands.cpp`. `src/` - all the project's source code resides here. Project-Id-Version: Phobos 
Report-Msgid-Bugs-To: 
POT-Creation-Date: 2021-08-16 14:17+0800
PO-Revision-Date: YEAR-MO-DA HO:MI+ZONE
Last-Translator: FULL NAME <EMAIL@ADDRESS>
Language: zh
Language-Team: zh <LL@li.org>
Plural-Forms: nplurals=1; plural=0
MIME-Version: 1.0
Content-Type: text/plain; charset=utf-8
Content-Transfer-Encoding: 8bit
Generated-By: Babel 2.9.1
 假设你已经成功克隆并构建了项目，理应拥有下列项目结构： 贡献 对贡献者的指导 项目结构 本页面介绍了如何帮助或为火卫一做出贡献，以及所有的实现方法。 `Commands/` - 快捷键源文件。每个新的快捷键类均继承自`PhobosCommandClass`（定义在`Commands.h`中）并且定义在一个分离的文件中，其中有一些方法，然后在`Commands.cpp`中注册。 `src/` - 所有项目源文件均位于此。` 