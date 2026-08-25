
# I/O (experimental)
The standard input and output are managed by a native syntax:

standard io error handler : `io::Error`

| instruction | syntax | info |
|-|-|-|
| output message | `output arg... \| args...`  `output::info arg... \| args...` | buffered
| output debug | `output::debug arg... \| args...` | buffered
| output warning | `output::warning arg... \| args...` | no buffered
| output error | `output::error arg... \| args...` | no buffered
| output critical | `output::critical arg... \| args...` | no buffered
| output critical | `output::flush arg... \| args...` | sugar syntax of `output "my text" \| io::flush`
| open file | `open my_file = "path" \| args...` | create variable, open any file with `open` args
| open as form | `open::MyForm my_file = "path" \| args...` | create variable, open with form operation and  `open` args
| close file | `close my_file` | close any file or form
| read file/stream | `read my_file in target \| args...` | read any file or form source and put in variable reference with `read` args
| input message | `input name = "Input message" \| args...` | create variable type `Result<T, io::Error>`, read any user input with `input` args, type is string by default
| input form | `input::MyForm name = "Input message" \| args...` | create variable type `Result<MyForm, io::Error>`, read any user input with `input` args, type is string by default
| write file/stream | `write source in my_file \| args...` | write any data in file or flow stream target with `write` args

## Arguments
Arguments are used to specify some I/O rules, used during the output or input operation
Consider arguments as conditions, if one condition is not respected, a io::error is returned in input variable result

Note: add `?` after argument to return directly `io::Error` in function return result

Note: Majority of arguments can be used directly as instruction e.g. `input::alpha name = "Enter your name: "`

| argument | syntax | info | instruction compatible |
|-|-|-|-|
| open read | `open::read` `open::r` | read only | yes
| open write | `open::write` `open::w` | write only | yes
| open write | `open::read_write` `open::rw` `open` | read write, default open mode | yes
| open add | `open::append` `open::app` | write at the end | yes
| open create | `open::create` | create file if not exists  | yes
| input alpha only | `input::alpha` `input::aA` | filter alphabet, input type `str` | yes
| input alphanumeric only | `input::alphanumeric` `input::aA09` | filter alphanumeric, input type `str` | yes
| input uppercase only | `input::uppercase` `input::A` | upper all, input type `str` | yes
| input lowercase only | `input::lowercase` `input::a` | lower all, input type `str` | yes
| input numeric only | `input::numeric` `input::_09` | filter numeric, input type `fsize` | yes
| input integer only | `input::integer` `input::int` | filter integer, input type `isize` | yes 
| input float only | `input::float` `input::float` | filter float, input type `fsize` | yes
| input boolean only | `input::bool` | filter boolean (`0` `1` `y` `o` `n`), input type `bool` | yes
| input size | `input::size N` | set the buffer size, can't be used as instruction | no
| input regex | `input::regex "\\b[a-zA-Z_][a-zA-Z0-9_]*\\b"` | custom regex, input type `str` | no
| output by line | `read::line_mode` | read by line | yes
| output by character | `read::character_mode` | read by character | yes
| output by word | `read::word_mode` | read by word | yes
| output by byte | `read::byte_mode` | read by byte | yes
| encoding | `encoding::utf8` | | no
| encoding | `encoding::utf32` | | no
| encoding | `encoding::ascii` | | no
| encoding | `encoding::byte` | | no
| flush buffer | `output::flush` | flush the flow buffer | yes

## custom io error
User can write a custom error text linked to the corresponding argument not respected
```
<argument> "custom error msg"`
```

> Note: or with return io::Error function `<argument>? "curstom error msg"`

The text is **NOT** writted in the output, </br>
but you can cumulate the io::error message with the writting in the output with literal text message annotation:



## I/O e.g.
**Input**
```
// try to get the age
fn ask_age_msg() -> Result<isize, io::Error>
  var count = 0
  loop {
    if input::integer in_age = "Write your age: " => return in_age
    else => output::error "Write only numbers!"

    if count++ > 3 => return io::error::worng_input_type("integer filter not respected")
  }
}
```
alternative:
```
// try to get the age
fn ask_age_msg() -> Result<isize, io::error> {
  input in_age = "Write your age: " | input::integer? "integer filter not respected"
  return in_age
}
```

**Read data**
```
// print every line in console
if open::read my_file = "/usr/config/somedata" {
  while read my_file in line | io::read::line {
    output line
  }
  output io::flush
} 
```

**Write data**
```
if open::write my_file = "/usr/config/somedata" {
  write "Hello World!" in my_file 
}
```

## Error handling
io::Error definition:
```
export IO {
  enum OperationIssue {
    open, open_read, open_write, open_append, open_create,
    input, input_alpha, input_uppercase, input_lowercase, input_alphanumeric, input_numeric, input_integer, input_float, input_bool, input_size, input_regex
    read, read_line_mode, read_character_mode, read_word_mode, read_byte_mode,
    encoding, encoding_utf8, encoding_utf32, encoding_ascii,
    end_of_file,
  }
  
  type Error<T>: Result<T, (EMismatch, str)>
}
```

## custom I/O
Users can write down their own I/O by using form operation overload, see [form](#form)


