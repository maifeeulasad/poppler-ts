// src/index.ts
import * as path from 'path';

// Load the native binding
const binding = require(path.join(__dirname, '../build/Release/poppler_binding.node'));

// Export the native binding
export const { Document, Page, version } = binding;

// Default export
export default binding;