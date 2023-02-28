
(transfer-examples)=
# Transfer Type examples with INI codes

## Table of Contents
- [Transfer Type examples with INI codes](#transfer-type-examples-with-ini-codes)
  - [Table of Contents](#table-of-contents)
  - [Examples by Tags](#examples-by-tags)
    - [Predefined INI Values](#predefined-ini-values)
    - [Basic Tags](#basic-tags)
      - [ConsiderArmor](#considerarmor)
      - [AffectHouses](#affecthouses)
      - [Target.Spread.IgnoreSelf](#targetspreadignoreself)
      - [Resource: money](#resource-money)
      - [Resource: experience](#resource-experience)
      - [Resource: health](#resource-health)
      - [Resource: ammo](#resource-ammo)
      - [Resource: gatlingrate](#resource-gatlingrate)
      - [Value.Type: current](#valuetype-current)
      - [Value.Type: missing](#valuetype-missing)
      - [Value.Type: total](#valuetype-total)
      - [Receive.SplitAmongOthers](#receivesplitamongothers)
    - [Intermediate Tags](#intermediate-tags)
      - [Direction: sourcetotarget and targettosource](#direction-sourcetotarget-and-targettosource)
      - [Direction: sourcetosource and targettotarget](#direction-sourcetosource-and-targettotarget)
      - [Direction: targettoextra and extratotarget](#direction-targettoextra-and-extratotarget)
      - [Extra.Spread.EpicenterIsSource](#extraspreadepicenterissource)
      - [Extra.Spread.IgnoreEpicenter](#extraspreadignoreepicenter)
      - [VeterancyMultiplier](#veterancymultiplier)
      - [Experience.PreventDemote](#experiencepreventdemote)
      - [Health.PreventKill](#healthpreventkill)
      - [GatlingRate.LimitStageChange](#gatlingratelimitstagechange)
    - [Advanced Tags](#advanced-tags)
      - [Receive.SentFactor: highest and average](#receivesentfactor-highest-and-average)
      - [Receive.SentFactor: sum and count](#receivesentfactor-sum-and-count)
      - [Send.PreventUnderflow](#sendpreventunderflow)
      - [Send.PreventOverflow](#sendpreventoverflow)
  - [Transfers in Use](#transfers-in-use)
    - [Experience on Repair](#experience-on-repair)
    - [Generals Hacker and Cash Hack](#generals-hacker-and-cash-hack)
    - [Paid/Free Field Promotion](#paidfree-field-promotion)
    - [Reset Units](#reset-units)
    - [Charge-staged Weapons](#charge-staged-weapons)
    - ["Blood Field"](#blood-field)
    - [Concentrator](#concentrator)
    - [Drain on "Undead" Brute](#drain-on-undead-brute)
    - [Experience Transfer](#experience-transfer)


## Examples by Tags
### Predefined INI Values

In all examples of tags default values are as written below unless stated otherwise.

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
Transfer.Types=GreenPlasmaTT
CellSpread=1.5
PercentAtMax=0.5

[LaserWH]
Verses=1%,1%,1%,1%,1%,1%,1%,1%,1%,1%,1%
Transfer.Types=RedLaserTT
CellSpread=1.5
PercentAtMax=0.5

[GreenRadWH]
Verses=1%,1%,1%,1%,1%,1%,1%,1%,1%,1%,1%
Transfer.Types=GreenPlasmaTT

[RedRadWH]
Verses=1%,1%,1%,1%,1%,1%,1%,1%,1%,1%,1%
Transfer.Types=RedLaserTT

[SpecialWH]
Verses=200%,200%,200%,50%,50%,50%,0%,0%,0%,0%,0%
Versus.deso=-100%
CellSpread=1.5
PercentAtMax=0.5

; -- TransferType List --
[TransferTypes]
0=GreenPlasmaTT
1=RedLaserTT
```

### Basic Tags

#### ConsiderArmor

![image](_static/images/transfertype/basic_target_armor.gif)

```ini
[GreenPlasmaTT]
Receive.Resource=health
Receive.Value=-100

[RedLaserTT]
Receive.Resource=health
Receive.Value=-100
Target.ConsiderArmor=yes
Target.VersusWarhead=SpecialWH
```

#### AffectHouses

![image](_static/images/transfertype/basic_target_houses.gif)

```ini
[GreenPlasmaTT]
Receive.Resource=health
Receive.Value=-100

[RedLaserTT]
Receive.Resource=health
Receive.Value=-100
Target.AffectHouses=enemy
```

#### Target.Spread.IgnoreSelf

![image](_static/images/transfertype/basic_target_ignoreself.gif)

```ini
[GreenPlasmaTT]
Receive.Resource=health
Receive.Value=-100

[RedLaserTT]
Receive.Resource=health
Receive.Value=-100
Target.Spread.IgnoreSelf=yes
```

#### Resource: money

![image](_static/images/transfertype/basic_resource_money.gif)

```ini
[GreenPlasmaTT]
Receive.Resource=money
Receive.Value=100
Money.Display.Receiver=yes
Money.Display.Receiver.Offset=0,-50

[RedLaserTT]
Receive.Resource=money
Receive.Value=-100
Money.Display.Receiver=yes
Money.Display.Receiver.Offset=0,-50
```

#### Resource: experience

![image](_static/images/transfertype/basic_resource_experience.gif)

```ini
[GreenPlasmaTT]
Receive.Resource=experience
Receive.Value=100

[RedLaserTT]
Receive.Resource=experience
Receive.Value=-100
```

#### Resource: health

![image](_static/images/transfertype/basic_resource_health.gif)

```ini
[GreenPlasmaTT]
Receive.Resource=health
Receive.Value=100

[RedLaserTT]
Receive.Resource=health
Receive.Value=-100
```

#### Resource: ammo

![image](_static/images/transfertype/basic_resource_ammo.gif)

```ini
[GreenPlasmaTT]
Receive.Resource=ammo
Receive.Value=4

[RedLaserTT]
Receive.Resource=ammo
Receive.Value=-4
```

#### Resource: gatlingrate

![image](_static/images/transfertype/basic_resource_gatlingrate.gif)

```ini
[GreenPlasmaTT]
Receive.Resource=gatlingrate
Receive.Value=8

[RedLaserTT]
Receive.Resource=gatlingrate
Receive.Value=-8
```

#### Value.Type: current

![image](_static/images/transfertype/basic_valuetype_current.gif)

```ini
[GreenPlasmaTT]
Receive.Resource=health
Receive.Value=50% ; note the % sign, otherwise it will become very high number
Receive.Value.Type=current

[RedLaserTT]
Receive.Resource=health
Receive.Value=-50% ; note the % sign, otherwise it will become very high number
Receive.Value.Type=current
```

#### Value.Type: missing

![image](_static/images/transfertype/basic_valuetype_missing.gif)

```ini
[GreenPlasmaTT]
Receive.Resource=health
Receive.Value=50% ; note the % sign, otherwise it will become very high number
Receive.Value.Type=missing

[RedLaserTT]
Receive.Resource=health
Receive.Value=-50% ; note the % sign, otherwise it will become very high number
Receive.Value.Type=missing
```

#### Value.Type: total

![image](_static/images/transfertype/basic_valuetype_total.gif)

```ini
[GreenPlasmaTT]
Receive.Resource=health
Receive.Value=50% ; note the % sign, otherwise it will become very high number
Receive.Value.Type=total

[RedLaserTT]
Receive.Resource=health
Receive.Value=-80% ; note the % sign, otherwise it will become very high number
Receive.Value.Type=total
```

#### Receive.Split

![image](_static/images/transfertype/basic_receive_split.gif)

```ini
[RedLaserTT]
Receive.Resource=health
Receive.Value=-450
Receive.Split=yes
```

### Intermediate Tags

#### Direction: sourcetotarget and targettosource

![image](_static/images/transfertype/inter_direct_s2t-t2s.gif)

```ini
[GreenPlasmaTT]
Direction=targettosource
Send.Resource=money
Send.Value=-100
Receive.Resource=health
Receive.Value=-100
Money.Display.Sender=yes
Money.Display.Sender.Offset=0,-50

[RedLaserTT]
Direction=sourcetotarget ; default value
Send.Resource=money
Send.Value=-100
Receive.Resource=health
Receive.Value=-100
Money.Display.Sender=yes
Money.Display.Sender.Offset=0,-50
```

#### Direction: sourcetosource and targettotarget

![image](_static/images/transfertype/inter_direct_s2s-t2t.gif)

```ini
[GreenPlasmaTT]
Direction=targettotarget
Send.Resource=money
Send.Value=-100
Receive.Resource=health
Receive.Value=-100
Money.Display.Sender=yes
Money.Display.Sender.Offset=0,-50

[RedLaserTT]
Direction=sourcetosource
Send.Resource=money
Send.Value=-100
Receive.Resource=health
Receive.Value=-100
Money.Display.Sender=yes
Money.Display.Sender.Offset=0,-50
```

#### Direction: targettoextra and extratotarget

![image](_static/images/transfertype/inter_direct_t2e-e2t.gif)

```ini
[SpecialWH]
CellSpread=0.1 ; extra only detects by CellSpread
PercentAtMax=1.0

[GreenPlasmaTT]
Direction=extratotarget
Extra.Warhead=SpecialWH
Send.Resource=money
Send.Value=-100
Receive.Resource=health
Receive.Value=-100
Money.Display.Sender=yes
Money.Display.Sender.Offset=0,-50

[RedLaserTT]
Direction=targettoextra
Extra.Warhead=SpecialWH
Send.Resource=money
Send.Value=-100
Receive.Resource=health
Receive.Value=-100
Money.Display.Sender=yes
Money.Display.Sender.Offset=0,-50
```

#### Extra.Spread.EpicenterIsSource

![image](_static/images/transfertype/inter_extra_source.gif)

```ini
[GreenPlasmaTT]
Direction=targettoextra
Extra.Warhead=SpecialWH
Extra.Spread.EpicenterIsSource=yes
Send.Resource=money
Send.Value=-100
Receive.Resource=health
Receive.Value=-100
Money.Display.Sender=yes
Money.Display.Sender.Offset=0,-50

[RedLaserTT]
Direction=targettoextra
Extra.Warhead=SpecialWH
Send.Resource=money
Send.Value=-100
Receive.Resource=health
Receive.Value=-100
Money.Display.Sender=yes
Money.Display.Sender.Offset=0,-50
```

#### Extra.Spread.IgnoreEpicenter

![image](_static/images/transfertype/inter_extra_ignore.gif)

```ini
[GreenPlasmaTT]
Direction=targettoextra
Extra.Warhead=SpecialWH
Extra.Spread.EpicenterIsSource=yes
Extra.Spread.IgnoreEpicenter=yes
Send.Resource=money
Send.Value=-100
Receive.Resource=health
Receive.Value=-100
Money.Display.Sender=yes
Money.Display.Sender.Offset=0,-50

[RedLaserTT]
Direction=targettoextra
Extra.Warhead=SpecialWH
Extra.Spread.IgnoreEpicenter=yes
Send.Resource=money
Send.Value=-100
Receive.Resource=health
Receive.Value=-100
Money.Display.Sender=yes
Money.Display.Sender.Offset=0,-50
```

#### Experience.PreventDemote

![image](_static/images/transfertype/inter_experience_demote.gif)

```ini
[GreenPlasmaTT]
Receive.Resource=experience
Receive.Value=-100
Experience.PreventDemote=yes

[RedLaserTT]
Receive.Resource=experience
Receive.Value=-100
```

#### Health.PreventKill

![image](_static/images/transfertype/inter_health_kill.gif)

```ini
[GreenPlasmaTT]
Receive.Resource=health
Receive.Value=-100
Health.PreventKill=yes

[RedLaserTT]
Receive.Resource=health
Receive.Value=-100
```

#### GatlingRate.LimitStageChange

![image](_static/images/transfertype/inter_gatling_limitstage.gif)

```ini
[GreenPlasmaTT]
Receive.Resource=gatlingrate
Receive.Value=500
GatlingRate.LimitStageChange=0

[RedLaserTT]
Receive.Resource=gatlingrate
Receive.Value=500
GatlingRate.LimitStageChange=2
```

### Advanced Tags

#### VeterancyMultiplier

![image](_static/images/transfertype/advan_veterancymultiplier.gif)

```ini
[GreenPlasmaTT]
Receive.Resource=money
Receive.Value=-100
VeterancyMultiplier.SourceOverReceiver=0.2,-2.0
Money.Display.Receiver=yes
Money.Display.Receiver.Offset=0,-50

[RedLaserTT]
Receive.Resource=money
Receive.Value=-100
VeterancyMultiplier.TargetOverTarget=0.2,-2.0
Money.Display.Receiver=yes
Money.Display.Receiver.Offset=0,-50
```

#### Receive.SentFactor: highest and average

![image](_static/images/transfertype/advan_factor_high-avg.gif)

```ini
[GreenPlasmaTT]
Direction=targettosource
Send.Resource=health
Send.Value=-100
Receive.Resource=money
Receive.Value=-100% ; negative because sent value is also negative (-1 * -1 = +1)
Receive.SentFactor=highest
Money.Display.Receiver=yes
Money.Display.Receiver.Offset=0,-50

[RedLaserTT]
Direction=targettosource
Send.Resource=health
Send.Value=-100
Receive.Resource=money
Receive.Value=-100% ; negative because sent value is also negative (-1 * -1 = +1)
Receive.SentFactor=average
Money.Display.Receiver=yes
Money.Display.Receiver.Offset=0,-50
```

#### Receive.SentFactor: sum and count

![image](_static/images/transfertype/advan_factor_sum-count.gif)

```ini
[GreenPlasmaTT]
Direction=targettosource
Send.Resource=health
Send.Value=-100
Receive.Resource=money
Receive.Value=-100% ; negative because sent value is also negative (-1 * -1 = +1)
Receive.SentFactor=sum
Money.Display.Receiver=yes
Money.Display.Receiver.Offset=0,-50

[RedLaserTT]
Direction=targettosource
Send.Resource=health
Send.Value=-100
Receive.Resource=money
Receive.Value=100
Receive.SentFactor=count
Money.Display.Receiver=yes
Money.Display.Receiver.Offset=0,-50
```

#### Send.PreventUnderflow

![image](_static/images/transfertype/advan_send_underflow.gif)

```ini
[GreenPlasmaTT]
Direction=targettosource
Send.Resource=health
Send.Value=-100
Send.PreventUnderflow=yes
Receive.Resource=money
Receive.Value=-100%
Receive.SentFactor=sum
Health.PreventKill=yes
Money.Display.Receiver=yes
Money.Display.Receiver.Offset=0,-50

[RedLaserTT]
Direction=targettosource
Send.Resource=health
Send.Value=-100
Receive.Resource=money
Receive.Value=-100%
Receive.SentFactor=sum
Health.PreventKill=yes
Money.Display.Receiver=yes
Money.Display.Receiver.Offset=0,-50
```

#### Send.PreventOverflow

![image](_static/images/transfertype/advan_send_overflow.gif)

```ini
[GreenPlasmaTT]
Send.Resource=health
Send.Value=100
Send.PreventOverflow=yes
Receive.Resource=money
Receive.Value=-100%
Receive.SentFactor=sum
Health.PreventKill=yes
Money.Display.Receiver=yes
Money.Display.Receiver.Offset=0,-50

[RedLaserTT]
Send.Resource=health
Send.Value=100
Receive.Resource=money
Receive.Value=-100%
Receive.SentFactor=sum
Health.PreventKill=yes
Money.Display.Receiver=yes
Money.Display.Receiver.Offset=0,-50
```

#### Special interaction

![image](_static/images/transfertype/advan_target_special.gif)

```ini
[LaserWH]
PercentAtMax=1.0

[RedLaserTT]
Direction=targettotarget
Send.Resource=health
Send.Value=-100%
Send.Value.Type=total
Send.PreventUnderflow=yes
Receive.Resource=money
Receive.Value=100%
Receive.SentFactor=average
Receive.Split=yes
Money.Display.Receiver=yes
Money.Display.Receiver.Offset=0,-50
```

## Transfers in Use

### Experience on Repair

### Generals Hacker Money
### Paid/Free Field Promotion
### Reset Units
### Charge-staged Weapons
### "Blood Field"
### Concentrator
### Drain on "Undead" Brute
### Experience Transfer