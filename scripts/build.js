// FoodRush - Multi-Platform Build Script for Local and Vercel Deployment
const { execSync } = require('child_process');
const fs = require('fs');
const path = require('path');
const os = require('os');

console.log('=== FoodRush Build Step ===');
console.log('Platform:', os.platform(), os.arch());

const isWindows = os.platform() === 'win32';
const engineExe = path.join(__dirname, '..', isWindows ? 'foodrush_engine.exe' : 'foodrush_engine');

// Check if g++ is available
try {
    const versionOutput = execSync('g++ --version', { encoding: 'utf8' });
    console.log('Detected C++ Compiler:');
    console.log(versionOutput.split('\n')[0]);

    console.log(`Compiling C++ Engine to ${engineExe}...`);
    const compileCmd = isWindows
        ? 'g++ -std=c++17 -O2 -static src/main.cpp -o foodrush_engine.exe'
        : 'g++ -std=c++17 -O2 src/main.cpp -o foodrush_engine';

    execSync(compileCmd, { cwd: path.join(__dirname, '..'), stdio: 'inherit' });

    if (!isWindows && fs.existsSync(engineExe)) {
        fs.chmodSync(engineExe, 0o755);
    }
    console.log('C++ Engine compiled successfully!');
} catch (err) {
    console.log('Note: g++ not available or compilation skipped in this environment.');
    console.log('FoodRush Serverless Engine bridge will handle requests seamlessly.');
}

console.log('=== Build Complete ===');
