# 📝 CollabEdit

A **Professional-Grade** real-time collaborative text editor built with **Node.js, Express, Socket.IO, and Quill.js**. This application replicates the core features of premium word processors, including real-time synchronization, rich-text formatting, and an integrated **AI Assistant**.

> Built for **Megathon 2025** 🚀

---

## ✨ Features

- **Real-Time Collaboration:** Multiple users can edit the same document simultaneously with near-zero latency using WebSocket synchronization.
- **Bespoke Premium UI:** A clean, high-contrast interface designed for focus. Features a centered A4 canvas, a functional top menu bar, and a dedicated formatting ribbon.
- **Rich Text Formatting:** Full support for Bold, Italics, Underline, Strikethrough, Custom Fonts, Font Sizes, Text Alignment, Lists, Links, Images, and **Dynamic Tables**.
- **✨ AI Writing Assistant:** A toggleable sidebar powered by **DeepSeek AI**. Ask it to generate, summarize, or format text, and insert results directly into your document with one click.
- **Robust Security:** Environment-safe architecture ensures API keys and secrets stay on the backend server.

---

## 🏗️ Architecture

- **Frontend:** Pure HTML5, CSS3 (Material-inspired), and Vanilla JS.
- **Rich Text Engine:** [Quill.js 2.0](https://quilljs.com/) for a reliable operational-transform document model.
- **Backend:** [Node.js](https://nodejs.org/) & [Express](https://expressjs.com/).
- **Synchronization:** [Socket.IO](https://socket.io/) for high-concurrency real-time broadcasting of document deltas.

---

## 🛠️ Prerequisites

- **Node.js** (v18 or higher recommended)
- **npm** (Node Package Manager)

---

## 🔨 Setup & Installation

**1. Clone & Install**
```bash
git clone <your-repo-url>
cd "Text Editor Final"
npm install
```

**2. Configure Environment**
Create a file named `.env` in the root directory and add your DeepSeek API Key:
```text
DEEPSEEK_API_KEY=sk-your-key-here
```

---

## 🚀 Running the Application

**1. Start the Server**
```bash
npm start
```
*The server will start at `http://localhost:3000`.*

**2. Open in Browser**
- Open `http://localhost:3000` in your browser.
- Open a second window to test the **Real-Time Collaboration**.

---

## 📁 Repository Structure

```
.
├── server.js             # Express & Socket.IO backend
├── package.json          # Node configuration
├── .env                  # Secret keys (ignored by git)
├── .gitignore            # Git exclusion rules
├── public/               # Frontend Assets
│   ├── index.html        # UI Structure
│   ├── style.css         # Premium Aesthetics
│   └── client.js         # Editor Logic & WebSocket handlers
├── archive/              # (Legacy) Original C source code for historical reference
└── README.md             # This documentation
```

---

## 📄 License

Development for **Megathon 2025**.
Repository maintained by **Aniket Goel**.
