// src/test.ts
import { Document, Page } from './index';
import * as fs from 'fs';

async function testPopplerBinding() {
  try {
    console.log('Testing Poppler Node.js binding...');
    
    // Test loading a PDF file
    const testPdfPath = './test.pdf'; // You'll need to provide a test PDF
    
    if (!fs.existsSync(testPdfPath)) {
      console.log('Creating a simple test by loading from buffer...');
      
      // You would need actual PDF buffer data here
      // This is just an example structure
      console.log('Please provide a test PDF file at ./test.pdf');
      return;
    }
    
    // Load document from file
    const doc = Document.loadFromFile(testPdfPath);
    console.log('✓ Document loaded successfully');
    
    // Get page count
    const pageCount = doc.getPageCount();
    console.log(`✓ Document has ${pageCount} pages`);
    
    // Check if document is locked
    const isLocked = doc.isLocked();
    console.log(`✓ Document locked: ${isLocked}`);
    
    // Get document metadata
    const metadata = doc.getMetadata();
    console.log('✓ Document metadata:', metadata);
    
    if (pageCount > 0) {
      // Get first page
      const page = doc.getPage(0);
      console.log('✓ First page obtained');
      
      // Get page size
      const size = page.getSize();
      console.log(`✓ Page size: ${size.width}x${size.height}`);
      
      // Get page rotation
      const rotation = page.getRotation();
      console.log(`✓ Page rotation: ${rotation}°`);
      
      // Extract text
      const text = page.getText();
      console.log(`✓ Text extracted (${text.length} characters)`);
      console.log('First 200 characters:', text.substring(0, 200));
      
      // Render page to image
      const image = page.renderToImage({
        dpi: 150,
        rotation: 0
      });
      console.log(`✓ Page rendered: ${image.width}x${image.height}, format: ${image.format}`);
      
      // Save rendered image (example - you'd need additional image processing)
      console.log(`✓ Image data size: ${image.data.length} bytes`);
    }
    
    console.log('\n🎉 All tests completed successfully!');
    
  } catch (error) {
    console.error('❌ Test failed:', error);
    process.exit(1);
  }
}

// Run the test
if (require.main === module) {
  testPopplerBinding();
}