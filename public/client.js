// client.js
// Handles Quill.js initialization, Socket.io syncing, and AI Chat interactions.

document.addEventListener('DOMContentLoaded', () => {
    // 1. Initialize Quill Editor
    // Expanded toolkit like Google Docs
    const toolbarOptions = [
        ['undo', 'redo', 'print'],                        // Emulated first line buttons
        [{ 'header': [1, 2, 3, 4, 5, 6, false] }, { 'font': [] }, { 'size': ['small', false, 'large', 'huge'] }],
        ['bold', 'italic', 'underline', 'strike'],        // Core styling
        [{ 'color': [] }, { 'background': [] }],          // Color selectors
        [{ 'align': [] }],                                // Alignment
        [{ 'list': 'ordered'}, { 'list': 'bullet' }],     // Lists
        [{ 'indent': '-1'}, { 'indent': '+1' }],          // Outdent/indent
        ['link', 'image', 'video'],                       // Media insertion
        ['table-insert'],                                 // Custom Table insert button
        ['clean']                                         // Clear formatting
    ];

    const quill = new Quill('#editor', {
        modules: {
            toolbar: {
                container: toolbarOptions,
                handlers: {
                    'table-insert': function() {
                        const table = this.quill.getModule('table');
                        if (table) table.insertTable(3, 3);
                        else console.warn("Table module not loaded");
                    }
                }
            },
            table: true // Enable Quill 2.0 built-in table module
        },
        theme: 'snow',
        placeholder: 'Start typing your document...'
    });

    // 2. Setup Socket.io Collaborative Sync
    const socket = io();
    const syncStatus = document.getElementById('sync-status-toast');
    let isRemoteUpdate = false;

    // Listen for text changes made by the user
    quill.on('text-change', (delta, oldDelta, source) => {
        if (source === 'user') {
            socket.emit('send-changes', delta);
            debounceSave();
        }
    });

    let saveTimeout;
    function debounceSave() {
        clearTimeout(saveTimeout);
        saveTimeout = setTimeout(() => {
            socket.emit('save-document', quill.getContents());
            syncStatus.textContent = "Saved to Drive ☁️";
            setTimeout(() => { syncStatus.textContent = "🟢 Connected"; }, 2000);
        }, 1000); 
        syncStatus.textContent = "Saving... 🔄";
    }

    socket.on('receive-changes', (delta) => {
        isRemoteUpdate = true;
        quill.updateContents(delta, 'api');
        isRemoteUpdate = false;
    });

    socket.on('load-document', (fullDocumentState) => {
        quill.setContents(fullDocumentState, 'api');
        quill.enable(); // Make editor active
    });

    socket.on('connect', () => {
        syncStatus.textContent = "🟢 Connected";
    });

    socket.on('disconnect', () => {
        syncStatus.textContent = "🔴 Disconnected";
    });

    // 3. Setup Right Sliding AI Panel Logic (Google Comments style)
    const aiToggleBtn = document.getElementById('ai-toggle-btn');
    const aiSidebar = document.getElementById('ai-sidebar');
    const aiCloseBtn = document.getElementById('close-ai-panel');

    // Toggle slide animation
    aiToggleBtn.addEventListener('click', () => {
        aiSidebar.classList.toggle('open');
    });

    aiCloseBtn.addEventListener('click', () => {
        aiSidebar.classList.remove('open');
    });

    const aiPromptInput = document.getElementById('ai-prompt');
    const sendBtn = document.getElementById('send-ai-btn');
    const responseText = document.getElementById('ai-response-text');
    const insertBtn = document.getElementById('ai-insert-btn');

    let lastAIResponse = "";

    async function sendPrompt() {
        const prompt = aiPromptInput.value.trim();
        if (!prompt) return;

        // Clear input immediately, add User Comment Card
        aiPromptInput.value = '';
        appendAICard('You', prompt, 'user-card');

        // Show generation state
        sendBtn.disabled = true;
        sendBtn.textContent = "...";
        insertBtn.disabled = true;

        const loadingId = 'loading-' + Date.now();
        appendAICard('System', 'Consulting DeepSeek AI...', 'system-card loading-indicator', loadingId);

        try {
            const res = await fetch('/api/chat', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ prompt })
            });
            const data = await res.json();
            
            // Remove loading message
            const loadingEl = document.getElementById(loadingId);
            if(loadingEl) loadingEl.remove();

            if (res.ok) {
                lastAIResponse = data.reply;
                appendAICard('AI Assistant', data.reply, 'system-card');
                insertBtn.disabled = false;
            } else {
                appendAICard('API Error', `${data.error} \n\n(Most likely Insufficient Balance / Payment Required 402).`, 'error-card');
            }
        } catch (err) {
            const loadingEl = document.getElementById(loadingId);
            if(loadingEl) loadingEl.remove();
            appendAICard('Network Error', 'Could not reach the Node.js server.', 'error-card');
        } finally {
            sendBtn.disabled = false;
            sendBtn.textContent = "💬 Send";
        }
    }

    // Clean modern generic cards
    function appendAICard(author, text, cardClass, id="") {
        const div = document.createElement('div');
        div.className = `ai-comment-card ${cardClass}`;
        if(id) div.id = id;

        const now = new Date();
        const timeStr = now.getHours() + ':' + (now.getMinutes() < 10 ? '0' : '') + now.getMinutes();

        div.innerHTML = `
            <div class="card-content">
                <strong>${author}</strong> <span class="time">${timeStr}</span>
                <p>${text.replace(/</g, "&lt;").replace(/>/g, "&gt;")}</p>
            </div>
        `;
        responseText.appendChild(div);
        responseText.scrollTop = responseText.scrollHeight;
    }

    sendBtn.addEventListener('click', sendPrompt);

    aiPromptInput.addEventListener('keydown', (e) => {
        if (e.key === 'Enter' && !e.shiftKey) {
            e.preventDefault();
            sendPrompt();
        }
    });

    insertBtn.addEventListener('click', () => {
        if (lastAIResponse) {
            const range = quill.getSelection(true);
            const index = range ? range.index : quill.getLength();
            quill.insertText(index, lastAIResponse + "\n", 'api');
            socket.emit('send-changes', { ops: [{ retain: index }, { insert: lastAIResponse + "\n" }] });
            
            appendAICard('System', '✓ Successfully injected text into your document.', 'system-card');
            insertBtn.disabled = true;
            lastAIResponse = "";
            
            setTimeout(() => {
                aiSidebar.classList.remove('open');
            }, 800);
        }
    });
});
