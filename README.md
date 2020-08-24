# Phobos

**Phobos** is a WIP project providing a set of new features and fixes for Yuri's Revenge based on [modified YRpp](https://github.com/Metadorius/YRpp) and [Syringe](https://github.com/Ares-Developers/Syringe) to allow injecting code. It's meant to accompany [Ares](https://github.com/Ares-Developers/Ares) rather than replace it, thus it won't introduce incompatibilities.


Building
--------

Windows:

0. Install Visual Studio with "Desktop development with C++" workload, "C++ Windows XP Support for VS 2017 (v141) tools" individual component and clone this repo recursively.
1. Open the project file in VS and compile the project.

Upon build completion place the resulting `Phobos.dll` in your YR directory and launch Syringe targeting your YR executable (usually `gamemd.exe`)


Credits
-------

- @Belonit aka Gluk-v48, @Metadorius aka Kerbiter, @misha135n2 - collaborators
- @TheAssemblyArmada (and @tomsons26 especially) - all-around help, assistance and guidance in reverse-engineering
- @Ares-Developers - creating YRpp, Syringe and Ares without which the project wouldn't exist
