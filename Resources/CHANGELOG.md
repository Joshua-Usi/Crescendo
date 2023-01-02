
Changelog format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
And this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html)
---
## [v0.1.0] - 2022-08-24
### Added
- Discord SDK integration
- OpenGL context (window support)
- Console::Output to output raw data
- Console::Ask to gather input from the console
- Synchronous file handling
### Optimised
- Changed std::function to std::function* in events to improve performance by optimising for cache lines
---
## [v0.0.1-a1] - 2022-08-17
### Added
- Logging library
- Events