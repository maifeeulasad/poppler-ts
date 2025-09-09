// src/types/index.ts

export interface PageSize {
  width: number;
  height: number;
  x: number;
  y: number;
}

export interface RenderOptions {
  dpi?: number;
  rotation?: number;
}

export interface RenderedImage {
  data: Buffer;
  width: number;
  height: number;
  bytesPerRow: number;
  format: string;
}

export interface DocumentMetadata {
  [key: string]: string;
}

export interface TextBox {
  text: string;
  bbox: {
    x: number;
    y: number;
    width: number;
    height: number;
  };
}

export interface Word {
  text: string;
  bbox: {
    x: number;
    y: number;
    width: number;
    height: number;
  };
}

export interface Line {
  text: string;
  bbox: {
    x: number;
    y: number;
    width: number;
    height: number;
  };
  words: Word[];
}

export interface PageJSON {
  page: {
    width: number;
    height: number;
    rotation: number;
  };
  lines: Line[];
}

export declare class Document {
  /**
   * Load a PDF document from file path
   */
  static loadFromFile(filepath: string, password?: string): Document;
  
  /**
   * Load a PDF document from buffer
   */
  static loadFromBuffer(buffer: Buffer, password?: string): Document;
  
  /**
   * Get the number of pages in the document
   */
  getPageCount(): number;
  
  /**
   * Get a specific page by index (0-based)
   */
  getPage(pageIndex: number): Page;
  
  /**
   * Get document metadata
   */
  getMetadata(): DocumentMetadata;
  
  /**
   * Check if the document is locked/encrypted
   */
  isLocked(): boolean;
  
  /**
   * Unlock the document with a password
   */
  unlock(password: string): boolean;
}

export declare class Page {
  /**
   * Get page dimensions and position
   */
  getSize(): PageSize;
  
  /**
   * Extract all text from the page
   */
  getText(): string;
  
  /**
   * Render the page to an image
   */
  renderToImage(options?: RenderOptions): RenderedImage;
  
  /**
   * Get page rotation in degrees
   */
  getRotation(): number;
  
  /**
   * Get page duration (for presentations)
   */
  getDuration(): number;
  
  /**
   * Get text boxes with coordinates
   */
  getTextBoxes(): TextBox[];
  
  /**
   * Export page content to JSON with detailed positioning
   */
  exportToJSON(): PageJSON;
}

export declare const version: string;