# atf

*atf* is an implementation of the scripting language known as *Advanced Title Formatting* or *Tagz*. The language has been used by some popular applications such as [Winamp](http://www.winamp.com), [foobar2000](https://www.foobar2000.org), [Mp3tag](http://www.mp3tag.de/en/) and [MusicBrainz Picard](https://picard.musicbrainz.org).

Basically, it converts this:

    [%artist% - ][%album% - ][$num(%track%,2) - ]$if2(%title%,$filepart(%filename%))

...to this:

    Iron Maiden - Brave New World - 04 - Blood Brothers

## Usage

***(atf is currently not in a usable state.)***

The library doesn't have any fields on its own. Applications have to override the `EvaluateField` virtual function in order to provide the data to corresponding fields:

```cpp
std::string CustomAtf::EvaluateField(const std::string& field) const {
  const std::map<std::string, std::string> fields{
    {"artist", "Iron Maiden"},
    {"album", "Brave New World"},
    {"track", "4"},
    {"title", "Blood Brothers"},
  };

  auto it = fields.find(field);
  return it != fields.end() ? it.second : std::string();
}
```

The library does provide you with a basic set of functions such as `if` and `len`. You may override the `EvaluateFunction` virtual function according to your application's needs:

```cpp
std::string CustomAtf::EvaluateFunction(
    const std::string& name, const std::vector<std::string>& params) const {
  std::string result;

  // $reverse(x)
  // Reverses the order of the characters in string x.
  if (name == "reverse" && params.size() == 1) {
    const auto& x = params.back();
    std::copy_backward(x.begin(), x.end(), result.end());
    return result;
  }

  return Atf::EvaluateFunction(name, params);
}
```

## Documentation

- [Winamp ATF Reference](https://web.archive.org/web/20130603113007/http://media.winamp.com/main/help/50/atf/atf.htm)
- [foobar2000 Title Formatting Reference](http://wiki.hydrogenaud.io/index.php?title=Foobar2000:Title_Formatting_Reference)
- [Mp3tag Scripting Functions](http://help.mp3tag.de/main_scripting.html)
- [MusicBrainz Picard Scripting](https://picard.musicbrainz.org/docs/scripting/)

## License

*atf* is licensed under the [MIT License](https://opensource.org/licenses/MIT).
