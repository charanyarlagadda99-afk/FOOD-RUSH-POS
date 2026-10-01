// ============================================================================
// FoodRush - Node.js Persistent Bridge Server
// Connects the Web UI to the persistent C++ Engine via stdin/stdout line protocol
// ============================================================================

const http = require('http');
const fs = require('fs');
const path = require('path');
const { spawn } = require('child_process');
const readline = require('readline');

const DEFAULT_PORT = process.env.PORT ? parseInt(process.env.PORT) : 3000;
let currentPort = DEFAULT_PORT;
const ENGINE_PATH = path.join(__dirname, '..', 'foodrush_engine.exe');
const PUBLIC_DIR = path.join(__dirname, '..', 'public');

console.log('============================================================');
console.log('FoodRush Persistent Bridge Server');
console.log(`Target C++ Engine Binary: ${ENGINE_PATH}`);
console.log('============================================================');

if (!fs.existsSync(ENGINE_PATH)) {
    console.error(`ERROR: Engine executable not found at ${ENGINE_PATH}`);
    console.error('Please compile the C++ engine first with: g++ -std=c++17 -O2 -static src/main.cpp -o foodrush_engine.exe');
    process.exit(1);
}

// Spawn the C++ engine ONCE as a persistent child process
const engineProcess = spawn(ENGINE_PATH, [], {
    cwd: path.join(__dirname, '..'),
    stdio: ['pipe', 'pipe', 'inherit']
});

engineProcess.on('error', (err) => {
    console.error('Failed to spawn C++ Engine process:', err);
});

engineProcess.on('exit', (code, signal) => {
    console.error(`C++ Engine exited with code ${code}, signal ${signal}`);
});

const rl = readline.createInterface({
    input: engineProcess.stdout,
    crlfDelay: Infinity
});

const pendingQueue = [];

rl.on('line', (line) => {
    const trimmed = line.trim();
    if (!trimmed) return;

    if (pendingQueue.length > 0) {
        const { resolve } = pendingQueue.shift();
        try {
            const parsed = JSON.parse(trimmed);
            resolve(parsed);
        } catch (e) {
            resolve({
                success: false,
                message: 'Failed to parse engine JSON response',
                raw: trimmed,
                handled_by: ['Server Line Protocol']
            });
        }
    }
});

function sendEngineCommand(commandString) {
    return new Promise((resolve, reject) => {
        pendingQueue.push({ resolve, reject });
        engineProcess.stdin.write(commandString.trim() + '\n');
    });
}

const MIME_TYPES = {
    '.html': 'text/html; charset=utf-8',
    '.css': 'text/css; charset=utf-8',
    '.js': 'application/javascript; charset=utf-8',
    '.json': 'application/json; charset=utf-8',
    '.svg': 'image/svg+xml',
    '.ico': 'image/x-icon',
    '.png': 'image/png',
    '.woff2': 'font/woff2'
};

function readJsonBody(req) {
    return new Promise((resolve) => {
        let body = '';
        req.on('data', chunk => body += chunk);
        req.on('end', () => {
            try {
                resolve(body ? JSON.parse(body) : {});
            } catch (e) {
                resolve({});
            }
        });
    });
}

const server = http.createServer(async (req, res) => {
    const parsedUrl = new URL(req.url, `http://${req.headers.host || 'localhost'}`);
    const pathname = parsedUrl.pathname;

    res.setHeader('Access-Control-Allow-Origin', '*');
    res.setHeader('Access-Control-Allow-Methods', 'GET, POST, OPTIONS');
    res.setHeader('Access-Control-Allow-Headers', 'Content-Type');

    if (req.method === 'OPTIONS') {
        res.writeHead(204);
        res.end();
        return;
    }

    // ------------------------------------------------------------------------
    // REST API ROUTING
    // ------------------------------------------------------------------------
    if (pathname.startsWith('/api/')) {
        res.setHeader('Content-Type', 'application/json; charset=utf-8');

        try {
            if (pathname === '/api/ping' && req.method === 'GET') {
                const resp = await sendEngineCommand('PING');
                res.writeHead(200);
                res.end(JSON.stringify(resp));
                return;
            }

            if (pathname === '/api/restaurants' && req.method === 'GET') {
                const resp = await sendEngineCommand('GET_RESTAURANTS');
                res.writeHead(200);
                res.end(JSON.stringify(resp));
                return;
            }

            if (pathname === '/api/menu' && req.method === 'GET') {
                const restId = parsedUrl.searchParams.get('restaurantId') || '0';
                const resp = await sendEngineCommand(`GET_MENU ${restId}`);
                res.writeHead(200);
                res.end(JSON.stringify(resp));
                return;
            }

            if (pathname === '/api/search' && req.method === 'GET') {
                const query = parsedUrl.searchParams.get('q') || '';
                const resp = await sendEngineCommand(`SEARCH_DISH ${query}`);
                res.writeHead(200);
                res.end(JSON.stringify(resp));
                return;
            }

            if (pathname === '/api/cart' && req.method === 'GET') {
                const zone = parsedUrl.searchParams.get('zone') || '0';
                const express = parsedUrl.searchParams.get('express') || '0';
                const resp = await sendEngineCommand(`CART_VIEW ${zone} ${express}`);
                res.writeHead(200);
                res.end(JSON.stringify(resp));
                return;
            }

            if (pathname === '/api/cart/add' && req.method === 'POST') {
                const data = await readJsonBody(req);
                const resp = await sendEngineCommand(`CART_ADD ${data.itemId} ${data.quantity || 1}`);
                res.writeHead(200);
                res.end(JSON.stringify(resp));
                return;
            }

            if (pathname === '/api/cart/remove' && req.method === 'POST') {
                const data = await readJsonBody(req);
                const resp = await sendEngineCommand(`CART_REMOVE ${data.itemId}`);
                res.writeHead(200);
                res.end(JSON.stringify(resp));
                return;
            }

            if (pathname === '/api/cart/undo' && req.method === 'POST') {
                const resp = await sendEngineCommand('CART_UNDO');
                res.writeHead(200);
                res.end(JSON.stringify(resp));
                return;
            }

            if (pathname === '/api/cart/clear' && req.method === 'POST') {
                const resp = await sendEngineCommand('CART_CLEAR');
                res.writeHead(200);
                res.end(JSON.stringify(resp));
                return;
            }

            if (pathname === '/api/coupon' && req.method === 'POST') {
                const data = await readJsonBody(req);
                const resp = await sendEngineCommand(`APPLY_COUPON ${data.code || ''}`);
                res.writeHead(200);
                res.end(JSON.stringify(resp));
                return;
            }

            if (pathname === '/api/checkout' && req.method === 'POST') {
                const data = await readJsonBody(req);
                const name = (data.customerName || 'Guest').replace(/\s+/g, '_');
                const zone = data.zoneId || 0;
                const street = (data.street || 'MainStreet').replace(/\s+/g, '_');
                const isExpress = data.isExpress ? '1' : '0';
                const resp = await sendEngineCommand(`CHECKOUT ${name} ${zone} ${street} ${isExpress}`);
                res.writeHead(200);
                res.end(JSON.stringify(resp));
                return;
            }

            if (pathname === '/api/track' && req.method === 'GET') {
                const code = parsedUrl.searchParams.get('code') || '';
                const resp = await sendEngineCommand(`TRACK_ORDER ${code}`);
                res.writeHead(200);
                res.end(JSON.stringify(resp));
                return;
            }

            if (pathname === '/api/orders/simulate' && req.method === 'POST') {
                const data = await readJsonBody(req);
                const cmd = data.orderId ? `SIMULATE_NEXT_STAGE ${data.orderId}` : 'SIMULATE_NEXT_STAGE';
                const resp = await sendEngineCommand(cmd);
                res.writeHead(200);
                res.end(JSON.stringify(resp));
                return;
            }

            if (pathname === '/api/orders' && req.method === 'GET') {
                const resp = await sendEngineCommand('GET_ORDERS');
                res.writeHead(200);
                res.end(JSON.stringify(resp));
                return;
            }

            if (pathname === '/api/orders/cook' && req.method === 'POST') {
                const data = await readJsonBody(req);
                const cmd = data.orderId ? `COOK_ORDER ${data.orderId}` : 'COOK_ORDER';
                const resp = await sendEngineCommand(cmd);
                res.writeHead(200);
                res.end(JSON.stringify(resp));
                return;
            }

            if (pathname === '/api/orders/assign-rider' && req.method === 'POST') {
                const data = await readJsonBody(req);
                const resp = await sendEngineCommand(`ASSIGN_RIDER ${data.orderId}`);
                res.writeHead(200);
                res.end(JSON.stringify(resp));
                return;
            }

            if (pathname === '/api/orders/complete' && req.method === 'POST') {
                const data = await readJsonBody(req);
                const resp = await sendEngineCommand(`COMPLETE_ORDER ${data.orderId}`);
                res.writeHead(200);
                res.end(JSON.stringify(resp));
                return;
            }

            if (pathname === '/api/riders' && req.method === 'GET') {
                const resp = await sendEngineCommand('GET_RIDERS');
                res.writeHead(200);
                res.end(JSON.stringify(resp));
                return;
            }

            if (pathname === '/api/sales-matrix' && req.method === 'GET') {
                const resp = await sendEngineCommand('GET_SALES_MATRIX');
                res.writeHead(200);
                res.end(JSON.stringify(resp));
                return;
            }

            if (pathname === '/api/array-stats' && req.method === 'GET') {
                const resp = await sendEngineCommand('GET_ARRAY_STATS');
                res.writeHead(200);
                res.end(JSON.stringify(resp));
                return;
            }

            if (pathname === '/api/benchmark' && req.method === 'GET') {
                const resp = await sendEngineCommand('BENCHMARK');
                res.writeHead(200);
                res.end(JSON.stringify(resp));
                return;
            }

            if (pathname === '/api/compare-ds' && req.method === 'GET') {
                const iters = parsedUrl.searchParams.get('iters') || '50000';
                const resp = await sendEngineCommand(`COMPARE_DS ${iters}`);
                res.writeHead(200);
                res.end(JSON.stringify(resp));
                return;
            }

            if (pathname === '/api/inspect' && req.method === 'GET') {
                const resp = await sendEngineCommand('INSPECT_ENGINE');
                res.writeHead(200);
                res.end(JSON.stringify(resp));
                return;
            }

            res.writeHead(404);
            res.end(JSON.stringify({ success: false, message: 'Unknown API endpoint' }));
            return;
        } catch (err) {
            res.writeHead(500);
            res.end(JSON.stringify({ success: false, message: err.message }));
            return;
        }
    }

    // ------------------------------------------------------------------------
    // STATIC FILE SERVING
    // ------------------------------------------------------------------------
    let targetFile = pathname === '/' || pathname === '/admin' ? 'index.html' : pathname;
    let filePath = path.join(PUBLIC_DIR, targetFile);

    if (!filePath.startsWith(PUBLIC_DIR)) {
        res.writeHead(403);
        res.end('Access Denied');
        return;
    }

    fs.stat(filePath, (err, stats) => {
        if (err || !stats.isFile()) {
            // Fallback to index.html for SPA client-side routing
            filePath = path.join(PUBLIC_DIR, 'index.html');
        }

        const ext = path.extname(filePath).toLowerCase();
        const contentType = MIME_TYPES[ext] || 'application/octet-stream';

        res.writeHead(200, { 'Content-Type': contentType });
        const stream = fs.createReadStream(filePath);
        stream.pipe(res);
    });
});

function startServer(port) {
    server.listen(port, () => {
        console.log(`FoodRush server running at http://localhost:${port}`);
        console.log(`- Customer Storefront: http://localhost:${port}`);
        console.log(`- Admin Operations:   http://localhost:${port}#admin`);
        console.log('Press Ctrl+C to terminate.');
    });
}

server.on('error', (err) => {
    if (err.code === 'EADDRINUSE') {
        console.log(`Port ${currentPort} is busy, retrying on port ${currentPort + 1}...`);
        currentPort++;
        startServer(currentPort);
    } else {
        console.error('Server error:', err);
    }
});

startServer(currentPort);
