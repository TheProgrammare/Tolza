# Format String
formatted text literal:
```
"text ${variable} and other ${variable}"
```
same as: `"text " + variable.str() + "and other" + variable.str()`

## Format Specifier
you can use some parameter to specify the out formatation of variable formatted
```
[[<fill>]<align>][<sign>]["#"]["0"][<width>][<grouping_option>]["."<precision>][<type>]

```

use in format string like:
```
"duration ${<expression>:<format_specifier>} s"
```

## Format Conditional (experimental)
you can specify some reaction with a comparison from the expression returned value:
```
(<value>:"<return_text>", ...)
```
special comparison operator:
- other case `_:"<return_text>"`
- positive `+:"<return_text>"`
- negative `-:"<return_text>"`

In format string:
```
"Gender: ${lerp}:<format_conditional>"
```

e.g.
```
var messages = 3
"You have ${messages}:(0:"no messages",1:"one message",_:${messages} "messages")"
var gender = "F"
"${gender}:("F":"She is online", "M":"He is online", _:"online")"
var account_balance = -2.0f
"${balance}:(+:"no depts", -:"some depts")"
```
