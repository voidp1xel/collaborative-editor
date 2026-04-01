const express = require('express');
const http = require('http');
const { Server } = require('socket.io');
const path = require('path');
const axios = require('axios');
require('dotenv').config();

const app = express();
const server = http.createServer(app);
const io = new Server(server);

// Middleware
app.use(express.static(path.join(__dirname, 'public')));
app.use(express.json());

// In-memory document state (in a real app, this would be backed by a DB)
// Using Quill.js deltas is the standard for collaborative editors, but for 
// simplicity without a Yjs/CRDT backend, we'll store the full HTML content 
// or the full Delta object. We'll store the latest Delta object here.
let documentState = null;

io.on('connection', (socket) => {
    console.log(`[+] User connected: ${socket.id}`);

    // Send the current document state to the newly connected user
    if (documentState) {
        socket.emit('load-document', documentState);
    }

    // Handle document changes from a client
    socket.on('send-changes', (delta) => {
        // Broadcast the delta to all *other* clients
        socket.broadcast.emit('receive-changes', delta);
    });

    // Handle the client saving the entire document state (to sync late joiners)
    socket.on('save-document', (data) => {
        documentState = data;
    });

    socket.on('disconnect', () => {
        console.log(`[-] User disconnected: ${socket.id}`);
    });
});

// AI Chat Endpoint (hides the API key from the frontend)
app.post('/api/chat', async (req, res) => {
    const { prompt } = req.body;
    const apiKey = process.env.DEEPSEEK_API_KEY;

    if (!apiKey) {
        return res.status(500).json({ 
            error: "Server configuration Error: DEEPSEEK_API_KEY environment variable is not set." 
        });
    }

    try {
        const response = await axios.post(
            'https://api.deepseek.com/chat/completions',
            {
                model: "deepseek-chat",
                messages: [{ role: "user", content: prompt }]
            },
            {
                headers: {
                    'Content-Type': 'application/json',
                    'Authorization': `Bearer ${apiKey}`
                },
                timeout: 30000
            }
        );

        if (response.data.choices && response.data.choices.length > 0) {
            res.json({ reply: response.data.choices[0].message.content });
        } else {
            res.status(500).json({ error: "Invalid response from AI." });
        }
    } catch (error) {
        console.error("AI API Error:", error.message);
        res.status(500).json({ 
            error: "Failed to reach AI service.", 
            details: error.message 
        });
    }
});

const PORT = process.env.PORT || 3000;
server.listen(PORT, () => {
    console.log(`=== CollabEdit Web Server ===`);
    console.log(`Listening on http://localhost:${PORT}`);
});
