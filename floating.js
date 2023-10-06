var loaded;
window.onload = function () { 
    loaded = true;
}

function paste() {
    window.opener.paste();
}

function hide() {
    if (loaded) {
        document.body.style.opacity = 0;
    }
}

function show() {
    if (loaded) {
        document.body.style.opacity = 1;
    }
}