# poppler-ts

A native Node.js binding for the Poppler PDF library, providing high-performance access to PDF documents through TypeScript/JavaScript.

## Features

- 📄 Load PDF documents from file paths or buffers
- 📊 Extract comprehensive document metadata
- 🔓 Handle password-protected PDFs with unlock functionality
- 📖 Extract plain text from pages
- � Get structured text boxes with precise coordinates
- 📋 Export page content to detailed JSON with word-level positioning
- �🖼️ Render pages to high-quality images (ARGB32 format)
- 📐 Get accurate page dimensions, rotation, and properties
- 🔄 Support for page rotation and duration (for presentations)
- ⚡ High-performance native C++ implementation
- 🔧 Full TypeScript support with type definitions

## Prerequisites

### System Dependencies

You need Poppler development libraries installed on your system:

#### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install libpoppler-cpp-dev build-essential
```

#### macOS (with Homebrew)
```bash
brew install poppler
```

#### Windows (with vcpkg)
```bash
vcpkg install poppler[cpp]:x64-windows
```

### Node.js Requirements
- Node.js >= 14.0.0
- Python 3.x (for node-gyp compilation)
- C++ compiler toolchain (GCC/Clang on Unix, MSVC on Windows)

## Installation

```bash
npm install poppler-ts
```

### Automatic Build Process

When you install the package, it will automatically attempt to build the native addon:

1. **System Check**: The installer checks for Poppler development libraries
2. **Native Build**: Compiles the C++ binding using node-gyp
3. **Verification**: Confirms the binary was created successfully

### Manual Build (if automatic build fails)

If the automatic build fails, you can build manually after installing system dependencies:

```bash
# Install system dependencies first, then:
npm run build:native
```

### Development Setup

If you're developing or need the full source:

1. Clone the repository:
```bash
git clone https://github.com/maifeeulasad/poppler-ts.git
cd poppler-ts
```

2. Install dependencies:
```bash
pnpm install
```

3. Build the native addon and TypeScript:
```bash
pnpm build
```

## Usage

### Basic Usage

```typescript
import { Document } from 'poppler-ts';
import * as fs from 'fs';

// Load a PDF document
const doc = Document.loadFromFile('./document.pdf');

// Or load from buffer
const buffer = fs.readFileSync('./document.pdf');
const doc2 = Document.loadFromBuffer(buffer);

// Get basic document info
console.log('Pages:', doc.getPageCount());
console.log('Is locked:', doc.isLocked());
console.log('Metadata:', doc.getMetadata());
```

### Working with Pages

```typescript
// Get the first page
const page = doc.getPage(0);

// Get page properties
const size = page.getSize();
console.log(`Page size: ${size.width}x${size.height} points`);

// Extract text
const text = page.getText();
console.log('Page text:', text);

// Get page rotation
const rotation = page.getRotation();
console.log(`Page rotation: ${rotation}°`);
```

### Advanced Text Extraction

```typescript
// Get text boxes with coordinates
const textBoxes = page.getTextBoxes();
console.log(`Found ${textBoxes.length} text boxes`);

textBoxes.forEach((box, index) => {
  console.log(`Box ${index + 1}: "${box.text}"`);
  console.log(`  Position: (${box.bbox.x}, ${box.bbox.y})`);
  console.log(`  Size: ${box.bbox.width}x${box.bbox.height}`);
});
```

### JSON Export with Detailed Positioning

```typescript
// Export page content to structured JSON
const pageJSON = page.exportToJSON();

console.log(`Page dimensions: ${pageJSON.page.width}x${pageJSON.page.height}`);
console.log(`Found ${pageJSON.lines.length} text lines`);

pageJSON.lines.forEach((line, index) => {
  console.log(`Line ${index + 1}: "${line.text}"`);
  console.log(`  Words: ${line.words.length}`);

  line.words.forEach((word, wordIndex) => {
    console.log(`    Word ${wordIndex + 1}: "${word.text}" at (${word.bbox.x}, ${word.bbox.y})`);
  });
});
```

### Image Rendering

```typescript
// Render page to image
const image = page.renderToImage({
  dpi: 150,
  rotation: 0
});

console.log(`Rendered image: ${image.width}x${image.height}`);
console.log(`Format: ${image.format}`);
console.log(`Data size: ${image.data.length} bytes`);

// Save as PNG (requires additional image processing library)
fs.writeFileSync('page.png', image.data);
```

### Handling Password-Protected PDFs

```typescript
// Check if document is locked
if (doc.isLocked()) {
  // Try to unlock with password
  const unlocked = doc.unlock('password123');
  if (!unlocked) {
    console.error('Failed to unlock document');
    return;
  }
}

// Now you can access the document content
const page = doc.getPage(0);
const text = page.getText();
```

## API Reference

### Document Class

#### Static Methods
- `Document.loadFromFile(filepath: string, password?: string): Document`
  - Load PDF from file path
  - Optional password for encrypted PDFs
- `Document.loadFromBuffer(buffer: Buffer, password?: string): Document`
  - Load PDF from memory buffer
  - Optional password for encrypted PDFs

#### Instance Methods
- `getPageCount(): number`
  - Returns the total number of pages
- `getPage(pageIndex: number): Page`
  - Returns a Page object for the specified index (0-based)
- `getMetadata(): DocumentMetadata`
  - Returns document metadata as key-value pairs
- `isLocked(): boolean`
  - Returns true if the document is password-protected
- `unlock(password: string): boolean`
  - Attempts to unlock the document with the provided password

### Page Class

#### Instance Methods
- `getSize(): PageSize`
  - Returns page dimensions and position
- `getText(): string`
  - Extracts all text content from the page
- `getTextBoxes(): TextBox[]`
  - Returns text boxes with precise coordinates
- `exportToJSON(): PageJSON`
  - Exports page content with detailed positioning information
- `renderToImage(options?: RenderOptions): RenderedImage`
  - Renders the page to an image buffer
- `getRotation(): number`
  - Returns page rotation in degrees (0, 90, 180, 270)
- `getDuration(): number`
  - Returns page duration for presentation PDFs

### Type Definitions

```typescript
interface PageSize {
  width: number;   // Page width in points
  height: number;  // Page height in points
  x: number;       // X offset (usually 0)
  y: number;       // Y offset (usually 0)
}

interface RenderOptions {
  dpi?: number;     // Resolution (default: 72)
  rotation?: number; // Additional rotation in degrees (default: 0)
}

interface RenderedImage {
  data: Buffer;        // Raw image data in ARGB32 format
  width: number;       // Image width in pixels
  height: number;      // Image height in pixels
  bytesPerRow: number; // Bytes per row
  format: string;      // Always "ARGB32"
}

interface TextBox {
  text: string;
  bbox: {
    x: number;
    y: number;
    width: number;
    height: number;
  };
}

interface Word {
  text: string;
  bbox: {
    x: number;
    y: number;
    width: number;
    height: number;
  };
}

interface Line {
  text: string;
  bbox: {
    x: number;
    y: number;
    width: number;
    height: number;
  };
  words: Word[];
}

interface PageJSON {
  page: {
    width: number;
    height: number;
    rotation: number;
  };
  lines: Line[];
}

interface DocumentMetadata {
  [key: string]: string; // Key-value pairs of metadata
}
```

## Build Scripts

- `pnpm build` - Build both native addon and TypeScript compilation
- `pnpm build:native` - Build only the native C++ addon using node-gyp
- `pnpm build:ts` - Compile only TypeScript files
- `pnpm clean` - Clean build artifacts and native addon
- `pnpm test` - Run the test suite with a sample PDF
- `pnpm example` - Run the example demonstrating advanced features
- `pnpm lint` - Check code style with ESLint
- `pnpm lint:fix` - Automatically fix linting issues

## Testing

The project includes comprehensive tests that demonstrate all functionality:

1. **Basic document operations**: Loading, page count, metadata
2. **Page operations**: Size, rotation, text extraction
3. **Advanced features**: Text boxes, JSON export, image rendering
4. **Password protection**: Document locking/unlocking

To run tests:

```bash
# Place a test PDF file at ./test.pdf
pnpm test
```

For advanced feature examples:

```bash
pnpm example
```

## Project Structure

```
├── src/
│   ├── cpp/                 # C++ native binding source
│   │   ├── poppler_binding.cpp  # Main binding file
│   │   ├── document.h       # Document class header
│   │   ├── document.cpp     # Document class implementation
│   │   ├── page.h           # Page class header
│   │   └── page.cpp         # Page class implementation
│   ├── types/               # TypeScript type definitions
│   │   └── index.ts         # Complete API type definitions
│   ├── index.ts             # Main TypeScript entry point
│   ├── test.ts              # Comprehensive test suite
│   └── example.ts           # Advanced features demonstration
├── dist/                    # Compiled JavaScript output
├── build/                   # Native addon build artifacts
│   └── Release/
│       └── poppler_binding.node
├── .github/workflows/       # CI/CD workflows
│   ├── lint.yml            # Linting on push/PR
│   └── publish2npm.yaml    # NPM publishing on release
├── binding.gyp             # node-gyp build configuration
├── eslint.config.ts        # ESLint configuration
├── package.json            # Project configuration
├── tsconfig.json           # TypeScript configuration
└── README.md
```

## CI/CD

The project includes GitHub Actions workflows for:

- **Linting**: Runs on every push and pull request to ensure code quality
- **Publishing**: Automatically publishes to NPM when a release is created

Both workflows include the necessary system dependencies for building the native addon.

### Installation Issues

If the automatic build fails during `npm install`:

1. **Install system dependencies** for your platform (see Prerequisites section above)
2. **Manual build**: Run `npm run build:native` after installing dependencies
3. **Verify installation**: Check that `build/Release/poppler_binding.node` exists
4. **Clear cache**: Try `npm cache clean --force` and reinstall

## Troubleshooting

### Build Issues

1. **Missing Poppler libraries**:
   ```bash
   # Ubuntu/Debian
   sudo apt-get update
   sudo apt-get install libpoppler-cpp-dev build-essential

   # macOS
   brew install poppler

   # Windows
   vcpkg install poppler[cpp]:x64-windows
   ```

2. **node-gyp compilation errors**:
   - Ensure Python 3.x is installed and in PATH
   - On Windows, install Visual Studio Build Tools
   - Clear node-gyp cache: `npm explore node-gyp -- npm run clean`

3. **macOS path issues**:
   ```bash
   export PKG_CONFIG_PATH="/opt/homebrew/lib/pkgconfig:$PKG_CONFIG_PATH"
   pnpm run build:native
   ```

### Runtime Issues

1. **Module not found**: Ensure native addon was built:
   ```bash
   ls build/Release/poppler_binding.node
   ```

2. **PDF loading errors**: Check file permissions and PDF validity

3. **Memory issues**: Large PDFs may require more memory allocation

4. **Text extraction issues**: Some PDFs may have complex layouts

## Performance Notes

- Native C++ implementation provides excellent performance
- Image rendering at high DPI may consume significant memory
- Text extraction is optimized for speed
- JSON export includes detailed positioning for advanced use cases

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes with tests
4. Ensure linting passes: `pnpm lint`
5. Submit a pull request

## License

MIT License - see LICENSE file for details.

## Dependencies

### Runtime
- [node-addon-api](https://github.com/nodejs/node-addon-api) - Node.js C++ addon API
- [Poppler](https://poppler.freedesktop.org/) - PDF rendering library

### Development
- TypeScript for type safety
- ESLint for code quality
- node-gyp for native compilation
- ts-node for running TypeScript directly

## Acknowledgments

Built with the excellent [Poppler PDF library](https://poppler.freedesktop.org/) and [Node.js addon API](https://nodejs.org/api/addons.html).