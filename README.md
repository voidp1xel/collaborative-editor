# 📝 CollabEdit

![Node.js](https://img.shields.io/badge/Node.js-339933?style=for-the-badge&logo=nodedotjs&logoColor=white)
![Express.js](https://img.shields.io/badge/Express.js-000000?style=for-the-badge&logo=express&logoColor=white)
![Socket.io](https://img.shields.io/badge/Socket.io-010101?style=for-the-badge&logo=socketdotio&logoColor=white)
![HTML5](https://img.shields.io/badge/HTML5-E34F26?style=for-the-badge&logo=html5&logoColor=white)
![CSS3](https://img.shields.io/badge/CSS3-1572B6?style=for-the-badge&logo=css3&logoColor=white)
![JavaScript](https://img.shields.io/badge/JavaScript-F7DF1E?style=for-the-badge&logo=javascript&logoColor=black)

**CollabEdit** is a **Professional-Grade**, real-time collaborative text editor built with **Node.js, Express, Socket.IO, and Quill.js**. This application replicates the core features of premium word processors, including real-time synchronization, rich-text formatting, and an integrated **AI Assistant** using DeepSeek AI.

> 🚀 **Built for Megathon 2025**

---

## ✨ Key Features

- 🤝 **Real-Time Collaboration:** Multiple users can edit the same document simultaneously with near-zero latency using WebSocket synchronization.
- 🎨 **Bespoke Premium UI:** A clean, high-contrast interface designed for focus. Features a centered A4 canvas, a functional top menu bar, and a dedicated formatting ribbon.
- ✍️ **Rich Text Formatting:** Full support for Bold, Italics, Underline, Strikethrough, Custom Fonts, Font Sizes, Text Alignment, Lists, Links, Images, and **Dynamic Tables**.
- 🤖 **✨ AI Writing Assistant:** A toggleable sidebar powered by **DeepSeek AI**. Ask it to generate, summarize, or format text, and insert results directly into your document with one click.
- 🔒 **Robust Security:** Environment-safe architecture ensures API keys and secrets stay on the backend server, completely hidden from the client browser.

---

## 🏗️ Architecture & Code Details

The application is structured into a lightweight, high-performance Node.js backend and a responsive Vanilla JS frontend. 

### 🖥️ Backend (`server.js`)
- **Express.js Server:** Serves static files from the `public/` directory and handles HTTP routes.
- **WebSocket (Socket.IO):** Manages real-time bi-directional communication.
  - Listeners are set up for `connection`, `disconnect`, `send-changes`, and `save-document`.
  - When a user sends changes (`send-changes`), the server broadcasts the delta to all other connected clients using `socket.broadcast.emit()`.
  - Maintains an in-memory `documentState` to synchronize late-joiners immediately upon connection (`socket.emit('load-document')`).
- **AI Proxy Route (`/api/chat`):** A dedicated endpoint using `axios` to securely communicate with the DeepSeek API. This ensures the frontend never exposes the secret `DEEPSEEK_API_KEY`.

### 🌐 Frontend (`public/`)
- **`index.html`:** The core structural template containing the main editor container, a premium formatting toolbar, and the slide-out AI assistant sidebar.
- **`style.css`:** Material-inspired styling, featuring clean typography, smooth transitions, and a layout that mimics a physical A4 paper on a dark/premium background.
- **`client.js`:** 
  - Initializes the **Quill.js** editor instance with specific modules (toolbar, etc.).
  - Handles WebSocket events. It applies incoming diffs (deltas) seamlessly using Quill's underlying Operation Transform (OT) model.
  - Manages the logic for sending prompts to the `/api/chat` backend endpoint and feeding the AI responses back into the editor UI.

---

## 🛠️ Prerequisites

Make sure you have the following installed to run this project:
- **Node.js** (v18 or higher recommended)
- **npm** (Node Package Manager)

---

## 🚀 Setup & Installation

**1. Clone the repository & Install Dependencies**
```bash
git clone <your-repo-url>
cd "CollabEdit"
npm install
```

**2. Configure Environment Variables**
Create a file named `.env` in the root directory. You will need a DeepSeek API Key for the AI assistant to function.
```text
DEEPSEEK_API_KEY=your_deepseek_api_key_here
```

**3. Start the Server**
```bash
npm start
```
*The server will start at `http://localhost:3000`.*

**4. Start Testing!**
- Open `http://localhost:3000` in your primary browser window.
- Open a second window to test the **Real-Time Collaboration** capabilities.

---

## 📁 Repository Structure

```text
.
├── server.js             # Express API & Socket.IO WebSockets backend
├── package.json          # Node dependencies & scripts
├── .env                  # Secret keys (ignored by git - create yourself)
├── .gitignore            # Git exclusion rules
├── public/               # Frontend Assets served statically
│   ├── index.html        # UI Structure
│   ├── style.css         # Premium Aesthetics & Layout
│   └── client.js         # Quill.js setup, Editor Logic & WebSocket handlers
├── archive/              # (Legacy) Original C source code for historical reference
└── README.md             # Project documentation (You are here!)
```

---

## © License & Copyright

**Copyright © 2025 Aniket Goel. All Rights Reserved.**

This repository and all its contents (code, design, and assets) are the exclusive property of Aniket Goel. **This is my personal code and it cannot be copied, distributed, modified, or used in any way without my explicit, prior written permission.**
