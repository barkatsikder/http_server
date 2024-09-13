// Import necessary modules
const express = require('express');
const path = require('path');

const app = express();
const PORT = 8080;

// Middleware to serve static files from the current directory
app.use(express.static(path.join(__dirname)));

// Route to serve the homepage (index.html)
app.get('/', (req, res) => {
    res.sendFile(path.join(__dirname, 'index.html'));
});

// Route to handle the "about" page
app.get('/about', (req, res) => {
    res.send('<h1>About Us</h1><p>We rock!</p>');
});

// Route to handle PDF requests
app.get('/document.pdf', (req, res) => {
    res.sendFile(path.join(__dirname, 'document.pdf'));
});

// Route to handle 404 errors (file not found)
app.use((req, res, next) => {
    res.status(404).send('<h1>404 Not Found</h1><p>The requested resource was not found.</p>');
});

// Start the server
app.listen(PORT, () => {
    console.log(`Server running on port ${PORT}`);
});