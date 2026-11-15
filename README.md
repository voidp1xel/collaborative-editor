<div align="center">

# 📝 CollabEdit — Real-Time Collaborative Rich-Text Editor

**A low-latency multi-user document editor powered by Operational Transformation (Quill Deltas), WebSockets, and an integrated AI Writing Assistant.**

![Node.js](https://img.shields.io/badge/Node.js-339933?style=for-the-badge&logo=nodedotjs&logoColor=white)
![Express.js](https://img.shields.io/badge/Express.js-000000?style=for-the-badge&logo=express&logoColor=white)
![Socket.io](https://img.shields.io/badge/Socket.io-010101?style=for-the-badge&logo=socketdotio&logoColor=white)
![JavaScript](https://img.shields.io/badge/JavaScript-F7DF1E?style=for-the-badge&logo=javascript&logoColor=black)
![HTML5](https://img.shields.io/badge/HTML5-E34F26?style=for-the-badge&logo=html5&logoColor=white)
![CSS3](https://img.shields.io/badge/CSS3-1572B6?style=for-the-badge&logo=css3&logoColor=white)

> 🚀 **Built for Megathon 2025** — Evolved from a low-level POSIX TCP socket prototype (`archive/`) into a full-featured web-based collaborative document workspace.

</div>

---

## 📌 Overview

**CollabEdit** is a real-time collaborative rich-text editor engineered with **Node.js, Express, Socket.IO, and Quill.js**, featuring an integrated **AI Writing Assistant** powered by the DeepSeek API. Multiple concurrent clients can edit a shared document simultaneously with sub-frame delta propagation, while late-joining peers automatically hydrate from an authoritative server-side document state.

---

## ✨ Key Engineering Features

- 🤝 **Real-Time Delta Synchronization (Operational Transformation):** Instead of broadcasting full document snapshots on every keystroke, clients emit compact **Quill Delta** operations (`insert`, `delete`, `retain`) over WebSockets, minimizing network payload size and preventing cursor clobbering.
- 🔄 **Late-Joiner State Hydration:** The backend maintains an authoritative in-memory `documentState` snapshot so newly connected clients immediately receive the latest document state via the `load-document` event.
- 🎨 **Desktop-Grade A4 Workspace UI:** High-contrast layout featuring a centered A4 page canvas, top menu bar, formatting ribbon, and slide-out AI drawer.
- ✍️ **Rich-Text & Dynamic Table Engine:** Supports typography controls, custom fonts, alignments, lists, inline links/media, and dynamic table insertion.
- 🤖 **Secure Server-Side AI Proxy (`/api/chat`):** Integrates DeepSeek AI for text generation, summarization, and rewriting via a backend Express proxy route, keeping `DEEPSEEK_API_KEY` strictly isolated from client browsers.
- 🏛️ **Systems Evolution (`archive/`):** Preserves the original multi-threaded **C POSIX TCP socket** client-server prototype in `archive/` (`server.c`, `client.c`) alongside the modern WebSocket architecture.

---

## 🏗️ System Architecture & Data Flow

```text
┌───────────────────┐     send-changes (Delta)     ┌──────────────────────────┐
│   Client A (UI)   │ ───────────────────────────► │   Node.js + Socket.IO    │
│  Quill.js Engine  │ ◄─────────────────────────── │      Backend Server      │
└───────────────────┘   receive-changes (Delta)    │                          │
                                                   │  • In-Memory Doc State   │
┌───────────────────┐     load-document (Init)     │  • Broadcast Multiplexer │
│   Client B (UI)   │ ◄─────────────────────────── │  • /api/chat AI Proxy    │
│  Quill.js Engine  │ ───────────────────────────► │             │            │
└───────────────────┘     POST /api/chat           └─────────────┼────────────┘
                                                                 │ HTTPS (Bearer)
                                                                 ▼
                                                   ┌──────────────────────────┐
                                                   │     DeepSeek LLM API     │
                                                   └──────────────────────────┘
```

### Module Breakdown

| Component | File(s) | Technical Responsibility |
| :--- | :--- | :--- |
| **WebSocket & API Server** | `server.js` | Configures Express static serving, manages Socket.IO lifecycle (`connection`, `send-changes`, `save-document`, `disconnect`), caches `documentState`, and proxies `/api/chat` requests via `axios`. |
| **Client OT & Socket Engine** | `public/client.js` | Initializes the Quill.js editor instance, suppresses echo loops by filtering `"user"` vs `"api"` change sources, applies remote deltas via `quill.updateContents(delta)`, and drives the AI sidebar workflow. |
| **Workspace UI & Styling** | `public/index.html`, `public/style.css` | Implements the A4 document canvas, formatting toolbar, and responsive AI assistant drawer. |
| **Legacy C Socket Prototype** | `archive/` | Original C implementation using POSIX `socket()`, `bind()`, `listen()`, and `pthread` concurrency for terminal-based collaborative editing. |

---

## 🚀 Getting Started

### Prerequisites

- **Node.js** (v18+ recommended)
- **npm** (Node Package Manager)

### Installation & Local Setup

```bash
# 1. Clone the repository
git clone https://github.com/voidp1xel/collaborative-editor.git
cd collaborative-editor

# 2. Install dependencies
npm install

# 3. Configure environment variables
cp .env.example .env
# Edit .env and set your DEEPSEEK_API_KEY

# 4. Start the server
npm start
```

The server listens on **`http://localhost:3000`**. Open multiple browser tabs or windows at `http://localhost:3000` to observe real-time multi-client synchronization.

---

## 📁 Repository Structure

```text
collaborative-editor/
├── server.js             # Express HTTP server, Socket.IO hub & DeepSeek proxy
├── package.json          # Node dependencies & scripts
├── .env.example          # Environment variable template
├── .gitignore            # Git exclusion rules (.env, node_modules, .DS_Store)
├── public/               # Static frontend application
│   ├── index.html        # Document editor shell, ribbon & AI assistant drawer
│   ├── style.css         # A4 canvas & dark/high-contrast UI styling
│   └── client.js         # Quill.js Delta handlers, Socket.IO client & AI calls
├── archive/              # Initial C POSIX TCP socket collaborative prototype
└── README.md             # Project documentation
```

---

## © License & Copyright

**Copyright © 2025–2026 Aniket Goel. All Rights Reserved.**

This repository and all its contents (code, design, and assets) are the exclusive property of Aniket Goel. It may not be copied, distributed, modified, or used in any way without explicit prior written permission.
