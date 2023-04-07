// Create an INI code block with given `newId` as a child of `parent` node, with an optional `height`
function makeINICodeBlock(parent, newId, height) {
	let div1 = document.createElement("div");
	div1.className = "highlight-ini notranslate";
	if (height != null) {
		div1.style.height = `${height}px`;
		div1.style.overflow = "auto";
    }
	let div2 = document.createElement("div");
	div2.className = "highlight";
	let pre = document.createElement("pre");
	pre.id = newId;
	div2.appendChild(pre);
	div1.appendChild(div2);
	parent.appendChild(div1);
}

// Add an INI line to a code block node (a direct parent <pre> node)
// An INI line consists of a `key`, `value` and `comment`, all can be null
function addINILine(codeBlockNode, line) {
	if (line.key != null) {
		let na = document.createElement("span");
		na.className = "na";
		na.textContent = line.key;
		codeBlockNode.appendChild(na);
		let o = document.createElement("span");
		o.className = "o";
		o.textContent = "=";
		codeBlockNode.appendChild(o);
	}
	if (line.value != null) {
		let s = document.createElement("span");
		s.className = "s";
		s.textContent = line.value;
		codeBlockNode.appendChild(s);
	}
	if (line.comment != null) {
		if (line.key != null || line.value != null) {
			let w = document.createElement("span");
			w.className = "w";
			w.textContent = ' ';
			codeBlockNode.appendChild(w);
		}
		let c1 = document.createElement("span");
		c1.className = "c1";
		c1.textContent = line.comment;
		codeBlockNode.appendChild(c1);
	}
	let w = document.createElement("span");
	w.className = "w";
	w.textContent = '\n';
	codeBlockNode.appendChild(w);
}
