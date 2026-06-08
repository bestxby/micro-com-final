import { chromium } from 'playwright';
import pptxgen from 'pptxgenjs';
import fs from 'fs/promises';
import path from 'path';

async function main() {
  const slidesDir = path.resolve('演示文档/slides');
  const outFile = path.resolve('演示文档.pptx');

  const files = (await fs.readdir(slidesDir))
    .filter(f => f.endsWith('.html'))
    .sort();
    
  console.log(`Found ${files.length} slides. Capturing screenshots and generating PPTX...`);

  const browser = await chromium.launch();
  const page = await browser.newPage({ viewport: { width: 1920, height: 1080 } });

  const pres = new pptxgen();
  pres.layout = 'LAYOUT_WIDE'; // 16:9 widescreen layout

  // Create a temp folder for slide screenshots
  const tempDir = path.join(process.cwd(), 'temp_slides_img');
  await fs.mkdir(tempDir, { recursive: true });

  for (let i = 0; i < files.length; i++) {
    const f = files[i];
    const filePath = path.join(slidesDir, f);
    const url = 'file://' + filePath;

    console.log(`  [${i + 1}/${files.length}] Rendering ${f}...`);
    // Wait for fonts to load and animations to settle
    await page.goto(url, { waitUntil: 'networkidle' }).catch(() => page.goto(url));
    await page.waitForTimeout(1200); 

    // Take screenshot
    const imgPath = path.join(tempDir, `slide_${i}.png`);
    await page.screenshot({ path: imgPath });

    // Add to PPTX as slide background
    const slide = pres.addSlide();
    slide.background = { path: imgPath };
  }

  await browser.close();

  // Save PPTX
  try {
    await pres.writeFile({ fileName: outFile });
    console.log(`\n✓ Success! PPTX presentation generated at: ${outFile}`);
  } catch (err) {
    if (err.code === 'EBUSY') {
      const fallbackFile = path.resolve('演示文档_new.pptx');
      console.warn(`\n⚠️  Warning: ${outFile} is busy or locked (likely open in PowerPoint).`);
      console.log(`Saving to fallback path: ${fallbackFile}`);
      await pres.writeFile({ fileName: fallbackFile });
      console.log(`\n✓ Success! Fallback PPTX presentation generated at: ${fallbackFile}`);
    } else {
      throw err;
    }
  }

  // Cleanup temp files
  const tempFiles = await fs.readdir(tempDir);
  for (const tf of tempFiles) {
    await fs.unlink(path.join(tempDir, tf));
  }
  await fs.rmdir(tempDir);
}

main().catch(console.error);
