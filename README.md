# Slan Browser

Slan Browser is a lightweight, multi-tabbed desktop application built with C++ and WebView2, designed to provide quick access to AI-powered web services (e.g., ChatGPT, Grok, Gemini) with a modern, customizable UI featuring a splash screen and acrylic effects.

## Features
- **Multi-Tab Interface**: Access 17 AI and productivity websites in distinct tabs with custom colors.
- **Splash Screen**: Displays a PNG-based splash with shake and fade effects on startup.
- **Performance Optimized**: Leverages WebView2 with GPU acceleration and QUIC for fast page loads.
- **Customizable UI**: Supports acrylic blur, dark mode, and a sleek, professional design.
- **Right-Click Downloads**: Save images and videos directly via a custom context menu.

## Installation
1. **Prerequisites**:
   - Windows 10/11 (64-bit)
   - Visual Studio 2022 with C++ Desktop Development workload
   - NuGet package: `Microsoft.Web.WebView2` (version 1.0.2739.15 or later)
   - vcpkg for `stb` library (`vcpkg install stb`)

2. **Clone the Repository**:
   ```bash
   git clone https://github.com/yourusername/slan-browser.git
   cd slan-browser
   ```

3. **Set Up vcpkg**:
   ```bash
   vcpkg integrate install
   vcpkg install stb
   ```

4. **Build the Project**:
   - Open `Slan.be.sln` in Visual Studio.
   - Set the solution configuration to `Release` or `Debug` (x64).
   - Build the solution (`Ctrl+Shift+B`).

## Usage
1. Launch the application (`Slan.be.exe`).
2. The splash screen appears with a shake and fade effect.
3. Use the tab control to switch between AI services (e.g., ChatGPT, Grok).
4. Click **Home** to reload the current tab’s default URL or **Back** to navigate backward.
5. Right-click images or videos to download them directly.
6. Click the **Slan** button to visit [slan.be](http://www.slan.be).

## Contributing
We welcome contributions! To get started:
1. Fork the repository.
2. Create a branch (`git checkout -b feature/your-feature`).
3. Commit changes (`git commit -m "Add your feature"`).
4. Push to your fork (`git push origin feature/your-feature`).
5. Open a Pull Request with a clear description.

 
![Untitled](https://github.com/user-attachments/assets/cabea8ff-8976-4c23-9630-626c07bd3761)

6.
Please read [CONTRIBUTING.md](docs/CONTRIBUTING.md) for guidelines and [CODE_OF_CONDUCT.md](docs/CODE_OF_CONDUCT.md) for our community standards.

## License
This project is licensed under the MIT License. See [LICENSE](LICENSE) for details.


 <div class="fallback">
    Buy ME cofee, 
    <a href="https://coff.ee/www.nutz.club" target="_blank" rel="noopener">Buy ME cofee</a>.
  </div>


## Contact
For issues or suggestions, open an issue on GitHub or contact us via [Twitter](https://x.com/@tv4k3).



<div class="fallback">
    Buy ME cofee, 
    <a href="https://www.nutz.club" target="_blank" rel="noopener">Site WEB</a>.
  </div>
