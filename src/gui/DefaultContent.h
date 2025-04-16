#pragma once

namespace DefaultContent {
constexpr auto WELCOME_MARKDOWN = R"(# Welcome to the notes panel📝

This is a simple yet powerful markdown editor with live preview. Here's a quick overview of what you can do:

## Basic Formatting

You can write text in **bold** or *italic*, or even ***both***!
~You can strike through if you want, also ~

### Lists and Tasks

Here are some things you can try:
- Basic bullet points
- [ ] Unchecked tasks
- [x] Completed tasks
  - Nested items work too!

Numeric lists work like:
1. one, hit enter and it magically becomes...
2.  two :)

### Code Blocks

Inline code looks like `this`. You can also create code:

```cpp
int main() {
    std::cout << "Hello, World!" << std::endl;
    return 0;
}
```
I know the syntax highlighting is a bit ugly right now. Patience please! You can remove the language
annotation (`cpp`) to just do raw text in a block.

### Links and Resources

- Visit [GitHub](https://github.com) for more
- Learn more about [Markdown](https://www.markdownguide.org/)

## Tips & Shortcuts

1. Toggle between edit/preview with the "View Mode" button
2. Your notes auto-save after typing

### Table Example

| Feature | Support |
|---------|---------|
| Tables  | ✅      |
| Lists   | ✅      |
| Code    | ✅      |

> **Pro tip**: You can customize colors and styles in your profile directory!

Now, delete all this stuff and get note-taking~)";
}