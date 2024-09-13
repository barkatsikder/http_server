#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <thread>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>

#define PORT 8080

// Function to parse the request line, headers, and body
void parseHttpRequest(const std::string& request, std::string& method, std::string& path, std::string& http_version, std::map<std::string, std::string>& headers, std::string& body) {
    std::istringstream requestStream(request);
    std::string line;

    // Parse the request line (e.g., "GET / HTTP/1.1")
    std::getline(requestStream, line);
    std::istringstream requestLine(line);
    requestLine >> method >> path >> http_version;  // Extract method, path, and HTTP version

    // Parse the headers
    while (std::getline(requestStream, line) && line != "\r") {
        std::string key, value;
        std::istringstream headerLine(line);
        std::getline(headerLine, key, ':');
        std::getline(headerLine, value);
        if (!key.empty() && !value.empty()) {
            headers[key] = value;
        }
    }

    // If there is a Content-Length header, read the body
    if (headers.find("Content-Length") != headers.end()) {
        int contentLength = std::stoi(headers["Content-Length"]);
        body.resize(contentLength);
        requestStream.read(&body[0], contentLength);
    }
}

// Helper function to check if a string ends with a given suffix
bool endsWith(const std::string& str, const std::string& suffix) {
    return str.size() >= suffix.size() && 
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

// Function to determine the content type based on the file extension
std::string getContentType(const std::string& path) {
    if (endsWith(path, ".html")) return "text/html";
    if (endsWith(path, ".css")) return "text/css";
    if (endsWith(path, ".js")) return "application/javascript";
    if (endsWith(path, ".png")) return "image/png";
    if (endsWith(path, ".jpg")) return "image/jpeg";
    if (endsWith(path, ".gif")) return "image/gif";
    if (endsWith(path, ".pdf")) return "application/pdf";
    return "text/plain";  // Default to plain text if file type is unknown
}

// Function to read the contents of a file
std::string readFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);  // Open the file in binary mode to read all types of files
    if (!file.is_open()) {
        return "";
    }

    std::ostringstream contents;
    contents << file.rdbuf();  // Read the file into a string
    return contents.str();
}

// Function to handle each client connection in a new thread
void handleClient(int client_socket) {
    char buffer[30000] = {0};
    long valread = read(client_socket, buffer, 30000);

    // Convert the request to a string for easier parsing
    std::string request(buffer);
    std::cout << "Received request:\n" << request << std::endl;

    // Variables to store parsed data
    std::string method, path, http_version, body;
    std::map<std::string, std::string> headers;

    // Parse the HTTP request
    parseHttpRequest(request, method, path, http_version, headers, body);

    // Set default path to index.html if root (/) is requested
    if (path == "/") {
        path = "/index.html";
    }

    // Remove the leading '/' from the path to get the actual file path
    std::string filePath = "." + path;

    // Read the requested file from disk
    std::string fileContent = readFile(filePath);

    std::string response;
    if (!fileContent.empty()) {
        // If file is found, respond with its contents
        std::string contentType = getContentType(filePath);
        response = "HTTP/1.1 200 OK\r\n"
                   "Content-Type: " + contentType + "\r\n"
                   "Content-Length: " + std::to_string(fileContent.size()) + "\r\n"
                   "\r\n" + fileContent;
    } else {
        // If file is not found, respond with a 404 error
        std::string notFoundContent = "<h1>404 Not Found</h1><p>The requested resource was not found.</p>";
        response = "HTTP/1.1 404 Not Found\r\n"
                   "Content-Type: text/html\r\n"
                   "Content-Length: " + std::to_string(notFoundContent.size()) + "\r\n"
                   "\r\n" + notFoundContent;
    }

    // Send the response to the client
    write(client_socket, response.c_str(), response.size());

    // Close the socket
    close(client_socket);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Bind the socket to the network interface and port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attach socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    std::cout << "Server started on port " << PORT << std::endl;

    // Accept incoming connections and handle requests concurrently using threads
    while (true) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        // Create a new thread to handle the client
        std::thread clientThread(handleClient, new_socket);
        clientThread.detach();  // Detach the thread to allow it to run independently
    }

    return 0;
}