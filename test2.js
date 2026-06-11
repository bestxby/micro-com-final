<script>
document.querySelectorAll('.agenda-card').forEach((card, idx) => {
  const targets = [2, 4, 7, 10, 14, 17, 20];
  if (idx < targets.length) {
    card.addEventListener('click', (e) => {
      e.preventDefault();
      e.stopPropagation();
      if(window.parent) window.parent.postMessage({goto: targets[idx]}, '*');
    });
  }
});
</script>