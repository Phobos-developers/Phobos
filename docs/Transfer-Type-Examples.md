
(transfer-gifs)=
# Transfer Type examples with INI codes

## Predefined INI values

In all examples specific tags are as written below unless stated otherwise.

```ini
[General]
VeteranRatio=2.0

; -- Technos --
[PLASMDESO]
Strength=1000
Cost=500
Soylent=150
Primary=PlasmaBlast
Secondary=GreenDeploy

[LASERDESO]
Strength=1000
Cost=500
Soylent=150
Primary=LaserBlast
Secondary=RedDeploy

[ANYTANK]
Strength=500
Cost=250
Soylent=250

; -- Weapons --
[PlasmaBlast]
Damage=1
ROF=50
Warhead=PlasmaWH

[LaserBlast]
Damage=1
ROF=50
Warhead=LaserWH

; -- Radiations --
[GreenRad]
RadSiteWarhead=GreenRadWH

[RedRad]
RadSiteWarhead=RedRadWH

; -- Warheads --
[PlasmaWH]
Verses=1%,1%,1%,1%,1%,1%,1%,1%,1%,1%,1%
Transfer.Types=GreenPlasmaTransfer
CellSpread=1.5
PercentAtMax=0.5

[LaserWH]
Verses=1%,1%,1%,1%,1%,1%,1%,1%,1%,1%,1%
Transfer.Types=RedLaserTransfer
CellSpread=1.5
PercentAtMax=0.5

[GreenRadWH]
Verses=1%,1%,1%,1%,1%,1%,1%,1%,1%,1%,1%
Transfer.Types=GreenPlasmaTransfer

[RedRadWH]
Verses=1%,1%,1%,1%,1%,1%,1%,1%,1%,1%,1%
Transfer.Types=RedLaserTransfer

[SpecialWH]
Verses=200%,200%,200%,50%,50%,50%,0%,0%,0%,0%,0%
Versus.deso=-100%
```

## Basic usage

![image](_static/images/transfertype/basic_resource_money.gif)
*Resource=money*

```ini
[GreenPlasmaTransfer]
Direction=targettosource
Send.Resource=money
Send.Value=100
Money.Display=...

[RedLaserTransfer]
Direction=targettosource
Send.Resource=money
Send.Value=-100
Money.Display=...
```

![image](_static/images/transfertype/basic_resource_experience.gif)
*Resource=experience*

```ini
[GreenPlasmaTransfer]
Direction=targettosource
Send.Resource=experience
Send.Value=100

[RedLaserTransfer]
Direction=targettosource
Send.Resource=experience
Send.Value=-100
```

![image](_static/images/transfertype/basic_resource_health.gif)
*Resource=health*

```ini
[GreenPlasmaTransfer]
Direction=targettosource
Send.Resource=health
Send.Value=100

[RedLaserTransfer]
Direction=targettosource
Send.Resource=health
Send.Value=-100
```

![image](_static/images/transfertype/basic_resource_ammo.gif)
*Resource=ammo*

```ini
[GreenPlasmaTransfer]
Direction=targettosource
Send.Resource=ammo
Send.Value=4

[RedLaserTransfer]
Direction=targettosource
Send.Resource=ammo
Send.Value=-4
```

![image](_static/images/transfertype/basic_resource_gatlingrate.gif)
*Resource=gatlingrate*

```ini
[GreenPlasmaTransfer]
Direction=targettosource
Send.Resource=gatlingrate
Send.Value=8

[RedLaserTransfer]
Direction=targettosource
Send.Resource=gatlingrate
Send.Value=-8
```

![image](_static/images/transfertype/basic_valuetype_current.gif)
*Value.Type=current*

```ini
[GreenPlasmaTransfer]
Direction=targettosource
Send.Resource=health
Send.Value=50% ; note the % sign, otherwise it will become very high number
Send.Value.Type=current

[RedLaserTransfer]
Direction=targettosource
Send.Resource=health
Send.Value=-50% ; note the % sign, otherwise it will become very high number
Send.Value.Type=current
```

![image](_static/images/transfertype/basic_valuetype_missing.gif)
*Value.Type=missing*

```ini
[GreenPlasmaTransfer]
Direction=targettosource
Send.Resource=health
Send.Value=50% ; note the % sign, otherwise it will become very high number
Send.Value.Type=missing

[RedLaserTransfer]
Direction=targettosource
Send.Resource=health
Send.Value=-50% ; note the % sign, otherwise it will become very high number
Send.Value.Type=missing
```

![image](_static/images/transfertype/basic_valuetype_total.gif)
*Value.Type=total*

```ini
[GreenPlasmaTransfer]
Direction=targettosource
Send.Resource=health
Send.Value=50% ; note the % sign, otherwise it will become very high number
Send.Value.Type=total

[RedLaserTransfer]
Direction=targettosource
Send.Resource=health
Send.Value=-50% ; note the % sign, otherwise it will become very high number
Send.Value.Type=total
```

![image](_static/images/transfertype/basic_value_flatlimits.gif)
*Value.FlatLimits*

```ini
[RedLaserTransfer]
Direction=targettosource
Send.Resource=health
Send.Value=-100
; FlatLimits retains minimum,maximum order in numbers only
; in negative values first value is negative maximum, second is negative minimum
Send.Value.FlatLimits=-60,-50 ; -60 is maximum possible damage, -50 is minimum
```

## Intermediate usage

![image](_static/images/transfertype/inter_target_armor.gif)
*Target.ConsiderArmor*

```ini
[GreenPlasmaTransfer]
Direction=targettosource
Send.Resource=health
Send.Value=-100

[RedLaserTransfer]
Direction=targettosource
Send.Resource=health
Send.Value=-100
Target.ConsiderArmor=yes
Target.VersusWarhead=SpecialWH
```

![image](_static/images/transfertype/inter_target_houses.gif)
*Target.AffectHouses*

```ini
[GreenPlasmaTransfer]
Direction=targettosource
Send.Resource=health
Send.Value=-100

[RedLaserTransfer]
Direction=targettosource
Send.Resource=health
Send.Value=-100
Target.AffectHouses=enemy
```

![image](_static/images/transfertype/inter_target_ignoreself.gif)
*Target.Spread.IgnoreSelf*

```ini
[GreenPlasmaTransfer]
Direction=targettosource
Send.Resource=health
Send.Value=-100

[RedLaserTransfer]
Direction=targettosource
Send.Resource=health
Send.Value=-100
Target.Spread.IgnoreSelf=yes
```

![image](_static/images/transfertype/inter_veterancymultiplier.gif)
*VeterancyMultiplier*

```ini
[GreenPlasmaTransfer]
Direction=targettosource
Send.Resource=money
Send.Value=-100
VeterancyMultiplier.SourceOverSender=0.2,-2.0
Money.Display=...

[RedLaserTransfer]
Direction=targettosource
Send.Resource=money
Send.Value=-100
VeterancyMultiplier.TargetOverTarget=0.2,-2.0
Money.Display=...
```

![image](_static/images/transfertype/inter_experience_demote.gif)
*Experience.PreventDemote*

```ini
[GreenPlasmaTransfer]
Direction=targettosource
Send.Resource=experience
Send.Value=-100
Experience.PreventDemote=yes

[RedLaserTransfer]
Direction=targettosource
Send.Resource=experience
Send.Value=-100
```

![image](_static/images/transfertype/inter_health_kill.gif)
*Health.PreventKill*

```ini
[GreenPlasmaTransfer]
Direction=targettosource
Send.Resource=health
Send.Value=-100
Health.PreventKill=yes

[RedLaserTransfer]
Direction=targettosource
Send.Resource=health
Send.Value=-100
```

![image](_static/images/transfertype/inter_gatling_limitstage.gif)
*Experience.PreventDemote*

```ini
[GreenPlasmaTransfer]
Direction=targettosource
Send.Resource=gatlingrate
Send.Value=500
GatlingRate.LimitStageChange=0

[RedLaserTransfer]
Direction=targettosource
Send.Resource=gatlingrate
Send.Value=500
GatlingRate.LimitStageChange=2
```

![image](_static/images/transfertype/inter_money_display.gif)
*Money.Display*

```ini
[GreenPlasmaTransfer]
Direction=targettosource
Send.Resource=money
Send.Value=-100
Money.Display.Sender=yes
Money.Display.Sender.Offset=0,-50

[RedLaserTransfer]
Direction=targettosource
Send.Resource=money
Send.Value=-100
Money.Display.Sender=yes
Money.Display.Sender.Offset=50,50
```
### TODO

## Advanced usage