# reference: https://www.gnu.org/software/sed/manual/sed.html#sed-scripts
# regexp playground (syntax may differ a bit): https://regexr.com

# from pre-0.3
s/^Gravity=0.*/Trajectory=Straight/I
s/^Rad\.NoOwner=(y.*|t.*|1)/; &\n; FIXME put RadHasOwner=no on used RadType/I
s/^Rad\.NoOwner=(n.*|f.*|1)/; &\n; FIXME put RadHasOwner=yes on used RadType/I
s/^Death\.NoAmmo(.*)/AutoDeath\.OnAmmoDepletion\1\n; FIXME set appropriate AutoDeath\.Behavior if not set/I
s/^Death\.Countdown(.*)/AutoDeath\.AfterDelay\1\n; FIXME set appropriate AutoDeath\.Behavior if not set/I
s/^Death\.Peaceful=(y.*|t.*|1)/AutoDeath\.Behavior=vanish/I
s/^Death\.Peaceful=(n.*|f.*|1)/AutoDeath\.Behavior=kill/I

# from 0.2.2.2
s/^PenetratesShield/Shield\.Penetrate/I
s/^BreaksShield/Shield\.Break/
