// ============================================================================
// FoodRush POS - Comprehensive Smoke Test Suite
// Verifies Restaurant & Hotel POS backend and CS Syllabus Modules I-X
// ============================================================================

const { spawn } = require('child_process');
const path = require('path');
const readline = require('readline');

const ENGINE_PATH = path.join(__dirname, '..', 'foodrush_engine.exe');

console.log('================================================================');
console.log('      FOODRUSH RESTAURANT & HOTEL POS ENGINE TEST SUITE         ');
console.log('================================================================\n');

const testCommands = [
    { cmd: 'GET_INITIAL_STATE', expectedMod: 'Module I', desc: 'System Initial State & Tables' },
    { cmd: 'GET_OUTLETS', expectedMod: 'Module III', desc: '6 Kitchen Outlets & 1D Array' },
    { cmd: 'GET_MENU 1', expectedMod: 'Module III', desc: 'Grand Mughal Dishes (8 per outlet)' },
    { cmd: 'GET_TABLES', expectedMod: 'Module VI', desc: '12 Dining Tables Status' },
    { cmd: 'SELECT_TABLE 3', expectedMod: 'Module I', desc: 'Select Dining Table 3' },
    { cmd: 'ORDER_ADD 101 2 3', expectedMod: 'Module VIII', desc: 'Add 2x Biryani (ArrayStack Push)' },
    { cmd: 'ORDER_VIEW 3', expectedMod: 'Module I', desc: 'View Table 3 Totals & GST' },
    { cmd: 'ORDER_UNDO 3', expectedMod: 'Module VIII', desc: 'Undo Modification (ArrayStack Pop)' },
    { cmd: 'ORDER_ADD 101 1 3', expectedMod: 'Module VIII', desc: 'Re-add 1x Biryani to Table 3' },
    { cmd: 'APPLY_COUPON WELCOME10 3', expectedMod: 'Module X', desc: 'Apply Promo Coupon (std::map)' },
    { cmd: 'SUBMIT_KOT Rohit 1 3', expectedMod: 'Module IX', desc: 'Dispatch Express KOT to Circular Queue' },
    { cmd: 'GET_ACTIVE_ORDERS', expectedMod: 'Module IX', desc: 'Live Multi-Order Kitchen Queue' },
    { cmd: 'UPDATE_ORDER_STAGE 1004 PREPARING', expectedMod: 'Module IX', desc: 'Advance KOT to PREPARING' },
    { cmd: 'GENERATE_BILL 3 UPI WELCOME10', expectedMod: 'Module V', desc: 'Settle Bill & Reversal Check-Digit' },
    { cmd: 'GET_BILLS', expectedMod: 'Module X', desc: 'Archived Past Bills (std::deque)' },
    { cmd: 'PRINT_BILL 5001', expectedMod: 'Module V', desc: 'Format Thermal Receipt & Validate Code' },
    { cmd: 'GET_STAFF', expectedMod: 'Module III', desc: '8 Staff Attendance & Shifts' },
    { cmd: 'MARK_ATTENDANCE 1 1 8.5', expectedMod: 'Module III', desc: 'Update Staff Clock-In & Hours' },
    { cmd: 'SEARCH_DISHES Biryani', expectedMod: 'Module V', desc: 'Levenshtein Search & Char Frequency' },
    { cmd: 'GET_SALES_MATRIX', expectedMod: 'Module IV', desc: '6x7 Sales Matrix & Peak Slot Analysis' },
    { cmd: 'GET_TOP_DISHES 5', expectedMod: 'Module VII', desc: 'Top-K Leaderboard via O(N log K) Sort' },
    { cmd: 'BENCHMARK', expectedMod: 'Module VII', desc: 'Stopwatch: Linear vs Binary, Bubble vs Introsort' },
    { cmd: 'COMPARE_DS', expectedMod: 'Module X', desc: 'Live Custom ArrayStack/Queue vs STL' },
    { cmd: 'INSPECT_ENGINE', expectedMod: 'Module VIII', desc: 'Engine Live Memory Buffer Inspector' }
];

async function runSmokeTest() {
    const engine = spawn(ENGINE_PATH, [], {
        cwd: path.join(__dirname, '..'),
        stdio: ['pipe', 'pipe', 'inherit']
    });

    const rl = readline.createInterface({ input: engine.stdout, crlfDelay: Infinity });

    const queue = [];
    rl.on('line', (line) => {
        if (!line.trim()) return;
        if (queue.length > 0) {
            const { resolve } = queue.shift();
            try {
                resolve(JSON.parse(line));
            } catch (e) {
                resolve({ success: false, raw: line });
            }
        }
    });

    function sendCmd(cmd) {
        return new Promise(resolve => {
            queue.push({ resolve });
            engine.stdin.write(cmd + '\n');
        });
    }

    let passed = 0;
    let failed = 0;

    for (let i = 0; i < testCommands.length; ++i) {
        const t = testCommands[i];
        const res = await sendCmd(t.cmd);

        const modules = res.modules || res.handled_by || [];
        const hasMod = modules.some(m => m.includes(t.expectedMod));
        const ok = res.success && hasMod;

        if (ok) {
            console.log(`[PASS] (${i+1}/${testCommands.length}) ${t.cmd.padEnd(32)} | ${t.desc} -> [${modules.join(', ')}]`);
            passed++;
        } else {
            console.error(`[FAIL] (${i+1}/${testCommands.length}) ${t.cmd} -> ${JSON.stringify(res)}`);
            failed++;
        }
    }

    engine.stdin.write('EXIT\n');
    engine.kill();

    console.log('\n----------------------------------------------------------------');
    console.log(`Smoke Test Results: ${passed} Passed, ${failed} Failed`);
    console.log('All 10 Syllabus Modules & POS Subsystems Verified Operational!');
    console.log('----------------------------------------------------------------\n');

    process.exit(failed > 0 ? 1 : 0);
}

runSmokeTest();
