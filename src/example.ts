// example.ts - demo
import { Document } from './index';
import * as fs from 'fs';
import { TextBox } from './types';

async function demonstrateNewFeatures() {
  try {
    // Load a PDF document
    const doc = Document.loadFromFile('./test.pdf');
    const page = doc.getPage(0);

    console.log('=== TEXT BOXES ===');
    const textBoxes = page.getTextBoxes();
    console.log(`Found ${textBoxes.length} text boxes`);

    // Show first few text boxes
    textBoxes.forEach((box: TextBox, i: number) => {
      console.log(`Text Box ${i + 1}: "${box.text}" at (${box.bbox.x.toFixed(1)}, ${box.bbox.y.toFixed(1)})`);
    });

    console.log('\n=== JSON EXPORT ===');
    const pageJSON = page.exportToJSON();
    console.log(`Page size: ${pageJSON.page.width}x${pageJSON.page.height}`);
    console.log(`Found ${pageJSON.lines.length} lines`);

    // Show first line details
    if (pageJSON.lines.length > 0) {
      const firstLine = pageJSON.lines[0];
      console.log(`First line: "${firstLine.text}"`);
      console.log(`Line bbox: (${firstLine.bbox.x.toFixed(1)}, ${firstLine.bbox.y.toFixed(1)}) ${firstLine.bbox.width.toFixed(1)}x${firstLine.bbox.height.toFixed(1)}`);
      console.log(`Words in line: ${firstLine.words.length}`);

      // Show first few words
      firstLine.words.forEach((word: TextBox, i: number) => {
        console.log(`  Word ${i + 1}: "${word.text}" at (${word.bbox.x.toFixed(1)}, ${word.bbox.y.toFixed(1)})`);
      });
    }

    // Save JSON to file
    fs.writeFileSync('page_content.json', JSON.stringify(pageJSON, null, 2));
    console.log('\nSaved page content to page_content.json');

  } catch (error) {
    console.error('Error:', error);
  }
}

demonstrateNewFeatures();