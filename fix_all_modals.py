import glob, re

# The sophisticated CSS from Slide 19 (but with bottom: 65px)
new_css = """
/* Floating Video Button at Bottom Right */
.view-video-btn {
  position: absolute;
  right: 80px;
  bottom: 65px;
  display: inline-flex;
  align-items: center;
  gap: 12px;
  padding: 16px 32px;
  background: var(--brand-primary);
  border: 1px solid rgba(25, 24, 22, 0.1);
  border-radius: 12px;
  color: #fff;
  font-size: 18px;
  font-weight: 800;
  box-shadow: 0 10px 25px rgba(204, 90, 1, 0.2);
  cursor: pointer;
  transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1);
  z-index: 100;
}
.view-video-btn:hover {
  transform: scale(1.05);
  background: var(--brand-secondary);
  box-shadow: 0 15px 35px rgba(15, 118, 110, 0.3);
}
.play-icon {
  font-size: 20px;
}

/* Fullscreen Video Modal */
.video-modal {
  position: fixed;
  top: 0; left: 0; width: 100%; height: 100%;
  background: rgba(25, 24, 22, 0.96);
  backdrop-filter: blur(16px);
  z-index: 9999;
  display: none;
  align-items: center;
  justify-content: center;
}
.video-modal-content {
  position: relative;
  width: 85%;
  max-width: 1400px;
  aspect-ratio: 16 / 9;
  border-radius: 24px;
  overflow: hidden;
  border: 1px solid var(--card-border);
  box-shadow: 0 25px 60px rgba(25, 24, 22, 0.5);
}
.video-modal-content video {
  width: 100%;
  height: 100%;
  display: block;
  outline: none;
}
.close-modal-btn {
  position: absolute;
  top: 32px;
  right: 32px;
  width: 54px;
  height: 54px;
  border-radius: 50%;
  background: rgba(252, 250, 247, 0.1);
  border: 1px solid rgba(252, 250, 247, 0.15);
  color: #FCFAF7;
  font-size: 32px;
  display: flex;
  align-items: center;
  justify-content: center;
  cursor: pointer;
  transition: all 0.2s ease;
  z-index: 10000;
}
.close-modal-btn:hover {
  background: var(--alert-red);
  border-color: var(--alert-red);
  transform: scale(1.1);
}
"""

# The HTML and Script
modal_html = """
<div class="video-modal" id="videoModal">
  <div class="close-modal-btn" id="closeModalBtn">&times;</div>
  <div class="video-modal-content">
    <video id="modalVideo" src="../shared/videos/security_demo.mp4" controls preload="metadata"></video>
  </div>
</div>
"""

script_html = """
<script>
  const openModalBtn = document.getElementById("openModalBtn");
  const videoModal = document.getElementById("videoModal");
  const closeModalBtn = document.getElementById("closeModalBtn");
  const modalVideo = document.getElementById("modalVideo");

  if(openModalBtn) {
      openModalBtn.addEventListener("click", () => {
        videoModal.style.display = "flex";
        modalVideo.play().catch(err => {
          console.log("Modal video autoplay failed or file not found:", err);
        });
      });
  }

  function closeModal() {
    videoModal.style.display = "none";
    modalVideo.pause();
  }

  if(closeModalBtn) {
      closeModalBtn.addEventListener("click", closeModal);
      videoModal.addEventListener("click", (e) => {
        if (e.target === videoModal) closeModal();
      });
  }
</script>
"""

# 1. Update tokens.css
tokens_path = glob.glob('*文档/shared/tokens.css')[0]
with open(tokens_path, 'r', encoding='utf-8') as f:
    css = f.read()

# Remove the old view-video-btn from tokens.css
if '/* Floating Video Button' in css:
    css = css[:css.find('/* Floating Video Button')]
elif '.view-video-btn' in css:
    start = css.find('.view-video-btn')
    end = css.find('}', start) + 1
    css = css[:start] + css[end:]

css += "\n" + new_css

with open(tokens_path, 'w', encoding='utf-8') as f:
    f.write(css)

# 2. Update all targets
targets = [
    'section4-02-hud-pages-overview.html',
    'section4-03-hud-ui-design.html',
    'section5-02-edge-algorithms.html',
    'section5-03-dirty-rects.html',
    'section6-02-home-security.html',
    'section6-03-wechat-logging.html'
]

for t in targets:
    paths = glob.glob(f'*文档/slides/{t}')
    if not paths: continue
    p = paths[0]
    with open(p, 'r', encoding='utf-8') as f:
        c = f.read()
    
    # Remove inline CSS in <style>
    if '/* Floating Video Button' in c:
        start_css = c.find('/* Floating Video Button')
        end_css = c.find('</style>', start_css)
        c = c[:start_css] + c[end_css:]
    
    # Inject Modal HTML if not present
    if 'id="videoModal"' not in c:
        c = c.replace('</body>', modal_html + '\n</body>')
        
    # Inject Script if not present
    if 'openModalBtn.addEventListener' not in c:
        c = c.replace('</body>', script_html + '\n</body>')
        
    # Add to file
    with open(p, 'w', encoding='utf-8') as f:
        f.write(c)

# Bump cache to v=6
for f in glob.glob('*文档/slides/*.html') + glob.glob('*文档/index.html'):
    with open(f, 'r', encoding='utf-8') as file:
        c = file.read()
    c = re.sub(r'tokens\.css\?v=\d+', 'tokens.css?v=6', c)
    with open(f, 'w', encoding='utf-8') as file:
        file.write(c)

print("SUCCESS: Full modal clone and CSS cleanup complete.")
