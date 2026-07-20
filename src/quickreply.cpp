#include "quickreply.h"

namespace QuickReply {

QString focusComposerScript() {
  // Try the footer composer first (the reply box), then any editable box that
  // is not the search field. Retried briefly because the chat may still be
  // opening when the notification is clicked.
  return QString::fromLatin1(R"JS(
(function () {
  'use strict';
  try {
    var tries = 0;
    var timer = setInterval(function () {
      if (++tries > 20) { clearInterval(timer); return; }
      var box =
        document.querySelector('footer [contenteditable="true"]') ||
        document.querySelector('div[data-tab="10"][contenteditable="true"]') ||
        document.querySelector('footer div[role="textbox"]');
      if (!box) return;
      clearInterval(timer);
      box.focus();
      // Put the caret at the end of any existing draft.
      var sel = window.getSelection && window.getSelection();
      if (sel && document.createRange) {
        var r = document.createRange();
        r.selectNodeContents(box);
        r.collapse(false);
        sel.removeAllRanges();
        sel.addRange(r);
      }
    }, 150);
  } catch (e) { /* never break the page */ }
})();
)JS");
}

} // namespace QuickReply
