// ============================================================================
// FoodRush - Frontend Application Logic
// Handles API calls, dynamic UI updates, and real-time Module Badge tracking
// ============================================================================

const API_BASE = '/api';

// Current active application state
let currentRestaurants = [];
let currentDishes = [];
let currentCart = { itemCount: 0, items: [], total: 0, undoStackDepth: 0 };
let activeRestaurantFilter = 0;

// Tab Switching
function switchTab(tabId) {
    document.querySelectorAll('.tab-pane').forEach(el => el.classList.remove('active'));
    document.querySelectorAll('.tab-btn').forEach(el => el.classList.remove('active'));

    const targetPane = document.getElementById(`tab-${tabId}`);
    if (targetPane) targetPane.classList.add('active');

    // Highlight button
    const buttons = document.querySelectorAll('.tab-btn');
    buttons.forEach(btn => {
        if (btn.getAttribute('onclick')?.includes(tabId)) {
            btn.classList.add('active');
        }
    });

    // Auto-refresh data on tab open
    if (tabId === 'cart') refreshCartView();
    if (tabId === 'kitchen') { loadOrders(); loadRiders(); }
    if (tabId === 'admin') { loadSalesMatrix(); loadArrayStats(); }
    if (tabId === 'inspector') loadEngineInspector();
}

// Module Tracker Badge updater
function updateModuleTracker(handledByList) {
    const container = document.getElementById('trackerBadges');
    if (!container || !handledByList || handledByList.length === 0) return;

    container.innerHTML = '';
    const badgeClasses = ['badge-primary', 'badge-secondary', 'badge-success', 'badge-info'];

    handledByList.forEach((mod, idx) => {
        const span = document.createElement('span');
        span.className = `badge ${badgeClasses[idx % badgeClasses.length]}`;
        span.textContent = mod;
        container.appendChild(span);
    });
}

// Toast notification helper
function showToast(message, isError = false) {
    const toast = document.getElementById('toast');
    if (!toast) return;
    toast.textContent = message;
    toast.style.borderColor = isError ? 'var(--accent-red)' : 'var(--accent-green)';
    toast.classList.add('show');
    setTimeout(() => toast.classList.remove('show'), 3500);
}

// Generic API caller with error handling and module tracking
async function callApi(endpoint, method = 'GET', body = null) {
    try {
        const options = { method, headers: { 'Content-Type': 'application/json' } };
        if (body) options.body = JSON.stringify(body);

        const res = await fetch(`${API_BASE}${endpoint}`, options);
        const json = await res.json();

        if (json.handled_by) {
            updateModuleTracker(json.handled_by);
        }

        return json;
    } catch (err) {
        console.error(`API Call failed for ${endpoint}:`, err);
        showToast(`Error connecting to engine: ${err.message}`, true);
        return { success: false, message: err.message };
    }
}

// ============================================================================
// RESTAURANTS & MENU LOGIC
// ============================================================================

async function loadRestaurants() {
    const resp = await callApi('/restaurants');
    if (resp.success && Array.isArray(resp.data)) {
        currentRestaurants = resp.data;
        renderRestaurantsList(currentRestaurants);
    }
}

function renderRestaurantsList(restaurants) {
    const container = document.getElementById('restaurantsList');
    if (!container) return;

    container.innerHTML = restaurants.map(r => `
        <div class="restaurant-card ${activeRestaurantFilter === r.id ? 'active' : ''}" onclick="filterMenuByRestaurant(${r.id})">
            <div class="rest-title">${r.name}</div>
            <div class="rest-meta">
                <span>🍽️ ${r.cuisine} Cuisine</span>
                <span class="rating-badge">★ ${r.rating.toFixed(1)}</span>
            </div>
            <div class="rest-meta" style="margin-top: 6px;">
                <span>📍 Zone ${r.zoneId}</span>
                <span style="color: var(--accent-green); font-weight: 600;">Open Now</span>
            </div>
        </div>
    `).join('');
}

async function loadMenu(restaurantId = 0) {
    const endpoint = restaurantId ? `/menu?restaurantId=${restaurantId}` : '/menu';
    const resp = await callApi(endpoint);
    if (resp.success && Array.isArray(resp.data)) {
        currentDishes = resp.data;
        renderDishesList(currentDishes, 'dishesList');
    }
}

function filterMenuByRestaurant(restId) {
    activeRestaurantFilter = restId;
    document.querySelectorAll('.filter-btn').forEach((btn, idx) => {
        btn.classList.toggle('active', idx === restId);
    });
    renderRestaurantsList(currentRestaurants);
    loadMenu(restId);
}

function renderDishesList(dishes, targetId) {
    const container = document.getElementById(targetId);
    if (!container) return;

    if (dishes.length === 0) {
        container.innerHTML = '<div class="empty-state">No dishes found matching this selection.</div>';
        return;
    }

    container.innerHTML = dishes.map(item => `
        <div class="dish-card">
            <div>
                <div class="dish-tags">
                    <span class="${item.isVeg ? 'tag-veg' : 'tag-nonveg'}">${item.isVeg ? '🌱 VEG' : '🥩 NON-VEG'}</span>
                    ${item.isSpicy ? '<span class="tag-spicy">🌶️ SPICY</span>' : ''}
                    ${item.isGlutenFree ? '<span class="tag-veg">🌾 GLUTEN FREE</span>' : ''}
                    ${item.isChefSpecial ? '<span class="tag-special">⭐ CHEF SPECIAL</span>' : ''}
                </div>
                <div class="dish-title">${item.name}</div>
                <div class="dish-category">${item.category} • ${item.calories} kcal</div>
            </div>
            <div class="dish-footer">
                <div>
                    <div class="dish-price">$${item.price.toFixed(2)}</div>
                    <div class="dish-stock">Stock: ${item.stock} left</div>
                </div>
                <button class="btn-add-cart" onclick="addToCart(${item.id})">+ Add to Cart</button>
            </div>
        </div>
    `).join('');
}

// ============================================================================
// SEARCH LOGIC [Module V: Strings]
// ============================================================================

function handleSearchKey(event) {
    if (event.key === 'Enter') {
        executeSearch();
    }
}

function quickSearch(term) {
    document.getElementById('searchInput').value = term;
    executeSearch();
}

async function executeSearch() {
    const input = document.getElementById('searchInput');
    const query = input.value.trim();
    if (!query) return;

    const resp = await callApi(`/search?q=${encodeURIComponent(query)}`);
    const header = document.getElementById('searchResultsHeader');
    const countSpan = document.getElementById('searchResultCount');

    if (resp.success && Array.isArray(resp.data)) {
        header.style.display = 'flex';
        countSpan.textContent = `${resp.data.length} matches for "${query}"`;
        renderDishesList(resp.data, 'searchResultsList');
        showToast(resp.message);
    }
}

// ============================================================================
// CART & UNDO LOGIC [Module VIII: Stack]
// ============================================================================

async function addToCart(itemId, quantity = 1) {
    const resp = await callApi('/cart/add', 'POST', { itemId, quantity });
    if (resp.success) {
        showToast(resp.message);
        currentCart = resp.data;
        updateCartBadge();
    } else {
        showToast(resp.message, true);
    }
}

async function removeFromCart(itemId) {
    const resp = await callApi('/cart/remove', 'POST', { itemId });
    if (resp.success) {
        showToast(resp.message);
        currentCart = resp.data;
        renderCartUI();
    }
}

async function cartUndo() {
    const resp = await callApi('/cart/undo', 'POST');
    const msgBox = document.getElementById('cartMessageBar');

    if (resp.success) {
        currentCart = resp.data;
        renderCartUI();
        showToast(resp.message);
        if (msgBox) {
            msgBox.textContent = `↩️ ${resp.message}`;
            msgBox.style.display = 'block';
            setTimeout(() => { msgBox.style.display = 'none'; }, 4000);
        }
    } else {
        showToast(resp.message, true);
    }
}

async function cartClear() {
    const resp = await callApi('/cart/clear', 'POST');
    if (resp.success) {
        currentCart = resp.data;
        renderCartUI();
        showToast('Cart and undo history cleared');
    }
}

async function refreshCartView() {
    const zone = document.getElementById('checkoutZone')?.value || 0;
    const express = document.getElementById('checkoutExpress')?.checked ? 1 : 0;

    const resp = await callApi(`/cart?zone=${zone}&express=${express}`);
    if (resp.success) {
        currentCart = resp.data;
        renderCartUI();
    }
}

function updateCartBadge() {
    const badge = document.getElementById('cartCountBadge');
    if (badge) badge.textContent = currentCart.itemCount || 0;
}

function renderCartUI() {
    updateCartBadge();

    // Update Undo Button badge with Stack depth
    const undoBadge = document.getElementById('undoBadge');
    if (undoBadge) {
        undoBadge.textContent = `Stack: ${currentCart.undoStackDepth || 0}`;
    }

    const container = document.getElementById('cartItemsContainer');
    if (!container) return;

    if (!currentCart.items || currentCart.items.length === 0) {
        container.innerHTML = '<div class="empty-state">Your cart is currently empty. Add dishes from the menu!</div>';
    } else {
        container.innerHTML = `
            <table class="cart-table">
                <thead>
                    <tr>
                        <th>Dish</th>
                        <th>Price</th>
                        <th>Qty</th>
                        <th>Total</th>
                        <th></th>
                    </tr>
                </thead>
                <tbody>
                    ${currentCart.items.map(item => `
                        <tr>
                            <td><strong>${item.name}</strong></td>
                            <td>$${item.price.toFixed(2)}</td>
                            <td>${item.quantity}</td>
                            <td><strong>$${item.lineTotal.toFixed(2)}</strong></td>
                            <td>
                                <button class="btn-remove-item" onclick="removeFromCart(${item.itemId})" title="Remove item">✕</button>
                            </td>
                        </tr>
                    `).join('')}
                </tbody>
            </table>
        `;
    }

    // Bill amounts
    document.getElementById('billSubtotal').textContent = `$${(currentCart.subtotal || 0).toFixed(2)}`;
    document.getElementById('billDiscount').textContent = `-$${(currentCart.discountAmount || 0).toFixed(2)}`;
    document.getElementById('billTax').textContent = `$${(currentCart.tax || 0).toFixed(2)}`;
    document.getElementById('billDelivery').textContent = `$${(currentCart.deliveryFee || 0).toFixed(2)}`;
    document.getElementById('billTotal').textContent = `$${(currentCart.total || 0).toFixed(2)}`;
}

async function applyCoupon() {
    const input = document.getElementById('couponInput');
    const code = input.value.trim();
    if (!code) return;

    const resp = await callApi('/coupon', 'POST', { code });
    if (resp.success) {
        showToast(resp.message);
        refreshCartView();
    } else {
        showToast(resp.message, true);
    }
}

async function submitCheckout() {
    if (currentCart.itemCount === 0) {
        showToast('Cannot checkout: Cart is empty!', true);
        return;
    }

    const name = document.getElementById('custName')?.value.trim() || 'Guest';
    const street = document.getElementById('custStreet')?.value.trim() || 'MainStreet';
    const zoneId = parseInt(document.getElementById('checkoutZone')?.value || 0);
    const isExpress = document.getElementById('checkoutExpress')?.checked || false;

    const resp = await callApi('/checkout', 'POST', {
        customerName: name,
        street: street,
        zoneId: zoneId,
        isExpress: isExpress
    });

    if (resp.success) {
        showToast(`🎉 ${resp.message}`);
        currentCart = { itemCount: 0, items: [], total: 0, undoStackDepth: 0 };
        renderCartUI();
        switchTab('kitchen');
    } else {
        showToast(resp.message, true);
    }
}

// ============================================================================
// KITCHEN & RIDERS LOGIC [Module IX: Queue]
// ============================================================================

async function loadOrders() {
    const resp = await callApi('/orders');
    if (resp.success && resp.data) {
        document.getElementById('metricKitchenQueue').textContent = resp.data.kitchenQueueSize || 0;
        document.getElementById('metricExpressQueue').textContent = resp.data.expressQueueSize || 0;

        const container = document.getElementById('ordersContainer');
        const orders = resp.data.orders || [];

        if (orders.length === 0) {
            container.innerHTML = '<div class="empty-state">No orders in system yet.</div>';
            return;
        }

        container.innerHTML = `
            <table class="data-table">
                <thead>
                    <tr>
                        <th>Order #</th>
                        <th>Customer</th>
                        <th>Type</th>
                        <th>Items</th>
                        <th>Total</th>
                        <th>Status</th>
                        <th>Rider / ETA</th>
                        <th>Action</th>
                    </tr>
                </thead>
                <tbody>
                    ${orders.map(o => `
                        <tr>
                            <td><strong>#${o.orderId}</strong></td>
                            <td>${o.customerName}</td>
                            <td>${o.isExpress ? '<span class="badge badge-primary">⚡ EXPRESS</span>' : '<span class="badge badge-info">STANDARD</span>'}</td>
                            <td>${o.itemCount} items</td>
                            <td><strong>$${o.totalAmount.toFixed(2)}</strong></td>
                            <td>
                                <span class="badge ${o.status === 'PLACED' ? 'badge-primary' : (o.status === 'READY_FOR_PICKUP' ? 'badge-info' : (o.status === 'DELIVERED' ? 'badge-success' : 'badge-secondary'))}">
                                    ${o.status}
                                </span>
                            </td>
                            <td>${o.riderName} (~${o.estimatedMinutes}m)</td>
                            <td>
                                ${o.status === 'PLACED' ? `<button class="btn-small" onclick="cookSpecificOrder(${o.orderId})">🍳 Cook</button>` : ''}
                                ${o.status === 'READY_FOR_PICKUP' ? `<button class="btn-small" onclick="assignRider(${o.orderId})">🛵 Assign Rider</button>` : ''}
                                ${o.status === 'OUT_FOR_DELIVERY' ? `<button class="btn-small" onclick="completeOrder(${o.orderId})">✅ Complete</button>` : ''}
                                ${o.status === 'DELIVERED' ? '<span class="text-green">Delivered</span>' : ''}
                            </td>
                        </tr>
                    `).join('')}
                </tbody>
            </table>
        `;
    }
}

async function cookNextOrder() {
    const resp = await callApi('/orders/cook', 'POST', {});
    if (resp.success) {
        showToast(resp.message);
        loadOrders();
    } else {
        showToast(resp.message, true);
    }
}

async function cookSpecificOrder(orderId) {
    const resp = await callApi('/orders/cook', 'POST', { orderId });
    if (resp.success) {
        showToast(resp.message);
        loadOrders();
    } else {
        showToast(resp.message, true);
    }
}

async function assignRider(orderId) {
    const resp = await callApi('/orders/assign-rider', 'POST', { orderId });
    if (resp.success) {
        showToast(resp.message);
        loadOrders();
        loadRiders();
    } else {
        showToast(resp.message, true);
    }
}

async function completeOrder(orderId) {
    const resp = await callApi('/orders/complete', 'POST', { orderId });
    if (resp.success) {
        showToast(resp.message);
        loadOrders();
        loadRiders();
    } else {
        showToast(resp.message, true);
    }
}

async function loadRiders() {
    const resp = await callApi('/riders');
    if (resp.success && Array.isArray(resp.data)) {
        const availableCount = resp.data.filter(r => r.isAvailable).length;
        document.getElementById('metricAvailableRiders').textContent = availableCount;

        const container = document.getElementById('ridersGrid');
        if (!container) return;

        container.innerHTML = resp.data.map(r => `
            <div class="rider-card">
                <strong>${r.name}</strong>
                <div style="font-size: 12px; color: var(--text-secondary); margin-top: 2px;">🛵 ${r.vehicle}</div>
                <div style="font-size: 11.5px; color: var(--text-muted);">Zone ${r.currentZone} • ${r.totalDeliveries} Deliveries</div>
                <span class="rider-status ${r.isAvailable ? 'status-available' : 'status-busy'}">
                    ${r.isAvailable ? '● Available' : '● On Delivery'}
                </span>
            </div>
        `).join('');
    }
}

// ============================================================================
// ADMIN & SALES MATRIX [Module IV: 2D Arrays]
// ============================================================================

async function loadSalesMatrix() {
    const resp = await callApi('/sales-matrix');
    if (resp.success && resp.data) {
        const m = resp.data;

        // Render Summary Row
        const summaryRow = document.getElementById('matrixStatsRow');
        if (summaryRow) {
            summaryRow.innerHTML = `
                <div class="metrics-row">
                    <div class="metric-card">
                        <span class="metric-label">Grand Platform Revenue</span>
                        <strong class="metric-value text-green">$${m.grandTotal.toFixed(2)}</strong>
                        <span class="metric-sub">Sum of all 4x7 elements</span>
                    </div>
                    <div class="metric-card">
                        <span class="metric-label">Peak Sales Single Slot</span>
                        <strong class="metric-value text-orange">$${m.peakSalesAmount.toFixed(2)}</strong>
                        <span class="metric-sub">Restaurant ${m.busiestRestaurant + 1} on Day: ${m.days[m.busiestDay]}</span>
                    </div>
                </div>
            `;
        }

        // Render Sales Matrix Table
        const salesContainer = document.getElementById('salesMatrixTableContainer');
        if (salesContainer) {
            const restNames = ["Bella Italia", "Tokyo Ramen", "Spice Symphony", "Burger Bistro"];
            let tableHtml = `
                <table class="data-table">
                    <thead>
                        <tr>
                            <th>Restaurant</th>
                            ${m.days.map(d => `<th>${d}</th>`).join('')}
                            <th>Weekly Total</th>
                        </tr>
                    </thead>
                    <tbody>
            `;

            for (let r = 0; r < 4; ++r) {
                tableHtml += `<tr><td><strong>${restNames[r]}</strong></td>`;
                for (let d = 0; d < 7; ++d) {
                    const isPeak = (r === m.busiestRestaurant && d === m.busiestDay);
                    tableHtml += `<td class="${isPeak ? 'matrix-cell-peak' : ''}">$${m.sales[r][d].toFixed(2)}</td>`;
                }
                tableHtml += `<td><strong class="text-green">$${m.restaurantWeeklyTotals[r].toFixed(2)}</strong></td></tr>`;
            }

            // Daily totals row
            tableHtml += `
                <tr style="background: var(--bg-card); font-weight: 700;">
                    <td>Daily Total</td>
                    ${m.dailyTotals.map(dt => `<td>$${dt.toFixed(2)}</td>`).join('')}
                    <td class="text-green">$${m.grandTotal.toFixed(2)}</td>
                </tr>
                </tbody></table>
            `;
            salesContainer.innerHTML = tableHtml;
        }

        // Render Distance Matrix Table
        const distContainer = document.getElementById('distanceMatrixTableContainer');
        if (distContainer) {
            let distHtml = `
                <table class="data-table">
                    <thead>
                        <tr>
                            <th>Zone From / To</th>
                            ${m.zones.map(z => `<th>${z}</th>`).join('')}
                        </tr>
                    </thead>
                    <tbody>
            `;
            for (let i = 0; i < m.zones.length; ++i) {
                distHtml += `<tr><td><strong>Zone ${i}: ${m.zones[i]}</strong></td>`;
                for (let j = 0; j < m.zones.length; ++j) {
                    distHtml += `<td>${m.distanceMatrix[i][j].toFixed(1)} km</td>`;
                }
                distHtml += `</tr>`;
            }
            distHtml += `</tbody></table>`;
            distContainer.innerHTML = distHtml;
        }
    }
}

async function loadArrayStats() {
    const resp = await callApi('/array-stats');
    if (resp.success && resp.data) {
        const d = resp.data;
        const container = document.getElementById('arrayStatsContainer');
        if (!container) return;

        container.innerHTML = `
            <div class="metrics-row" style="margin-top: 14px;">
                <div class="metric-card">
                    <span class="metric-label">Ratings 1D Array Stats</span>
                    <div style="margin-top: 6px; font-size: 13.5px;">
                        <div>Average Rating: <strong>${d.ratings.average.toFixed(2)} / 5.0</strong></div>
                        <div>Highest Rating: <strong>${d.ratings.max.toFixed(2)}</strong> (${d.ratings.bestRestaurant})</div>
                        <div>Lowest Rating: <strong>${d.ratings.min.toFixed(2)}</strong> (${d.ratings.lowestRestaurant})</div>
                    </div>
                </div>
                <div class="metric-card">
                    <span class="metric-label">Menu Prices 1D Array Stats</span>
                    <div style="margin-top: 6px; font-size: 13.5px;">
                        <div>Average Dish Price: <strong>$${d.prices.average.toFixed(2)}</strong></div>
                        <div>Most Expensive: <strong>$${d.prices.max.toFixed(2)}</strong> (${d.prices.priciestDish})</div>
                        <div>Cheapest Dish: <strong>$${d.prices.min.toFixed(2)}</strong> (${d.prices.cheapestDish})</div>
                        <div style="margin-top: 6px; font-size: 11.5px; color: var(--text-muted);">
                            Bubble Sorted Prices Sample: [${d.prices.sortedSample.join(', ')}]
                        </div>
                    </div>
                </div>
            </div>
        `;
    }
}

// ============================================================================
// ENGINE INSPECTOR LOGIC [Modules VIII, IX, X]
// ============================================================================

async function loadEngineInspector() {
    const resp = await callApi('/inspect');
    if (resp.success && resp.data) {
        const d = resp.data;

        // Stack Inspector
        const stackBadge = document.getElementById('inspStackBadge');
        if (stackBadge) stackBadge.textContent = `Depth: ${d.stackInspector.size} / ${d.stackInspector.capacity}`;

        const stackVis = document.getElementById('stackVisualizer');
        if (stackVis) {
            if (d.stackInspector.elements.length === 0) {
                stackVis.innerHTML = '<div class="empty-state">Stack is empty. Add/remove cart items to push frames.</div>';
            } else {
                stackVis.innerHTML = d.stackInspector.elements.map((frame, idx) => `
                    <div class="stack-frame ${idx === 0 ? 'stack-frame-top' : ''}">
                        <strong>[Top - ${idx}] Action: ${frame.type}</strong> | Item: ${frame.itemName} (ID: ${frame.itemId})
                        <span style="color: var(--text-muted); margin-left: 8px;">prev: ${frame.prevQty} ➔ new: ${frame.newQty}</span>
                    </div>
                `).join('');
            }
        }

        // Circular Queue Inspector
        const queueBadge = document.getElementById('inspQueueBadge');
        if (queueBadge) queueBadge.textContent = `${d.circularQueueInspector.count} / ${d.circularQueueInspector.capacity}`;

        const queueVis = document.getElementById('queueVisualizer');
        if (queueVis) {
            if (d.circularQueueInspector.elements.length === 0) {
                queueVis.innerHTML = '<div class="empty-state">Circular Kitchen Queue is currently empty.</div>';
            } else {
                queueVis.innerHTML = d.circularQueueInspector.elements.map((ordId, offset) => `
                    <div class="queue-slot ${offset === 0 ? 'queue-slot-front' : ''}">
                        <span>${offset === 0 ? '👉 FRONT:' : 'SLOT:'}</span>
                        <strong>Order #${ordId}</strong>
                    </div>
                `).join('');
            }
        }

        // Recently Viewed Dishes Stack
        const recentVis = document.getElementById('recentDishesVisualizer');
        if (recentVis) {
            if (d.recentlyViewedStack.items.length === 0) {
                recentVis.innerHTML = '<div class="empty-state">No dishes viewed yet.</div>';
            } else {
                recentVis.innerHTML = `
                    <div style="display: flex; gap: 8px; flex-wrap: wrap;">
                        ${d.recentlyViewedStack.items.map(dish => `
                            <span class="badge badge-secondary">ID #${dish.id}: ${dish.name}</span>
                        `).join('')}
                    </div>
                `;
            }
        }

        // STL Overview
        const stlContainer = document.getElementById('stlOverviewContainer');
        if (stlContainer) {
            stlContainer.innerHTML = `
                <div style="font-size: 13.5px; line-height: 1.8;">
                    <div><code>std::map&lt;string, double&gt;</code> Coupons Registered: <strong>${d.stlOverview.couponsInMap}</strong></div>
                    <div><code>std::set&lt;string&gt;</code> Unique Cuisines Registered: <strong>${d.stlOverview.cuisinesInSet}</strong></div>
                    <div><code>std::vector&lt;std::pair&lt;int, double&gt;&gt;</code> Promo Pairs: <strong>${d.stlOverview.dailySpecialPairs}</strong></div>
                </div>
            `;
        }
    }
}

// ============================================================================
// BENCHMARKS & STL COMPARISON [Modules VII, X]
// ============================================================================

async function runBenchmark() {
    const area = document.getElementById('benchmarkResultsArea');
    area.innerHTML = '<div class="empty-state">⏱️ Running C++ high-resolution benchmarks on 30,000 search elements and 2,500 sort elements...</div>';

    const resp = await callApi('/benchmark');
    if (resp.success && resp.data) {
        const b = resp.data;
        area.innerHTML = `
            <div class="benchmark-card-grid">
                <!-- Search Benchmark Card -->
                <div class="card">
                    <div class="card-header">
                        <h3>🔍 Search Benchmark (${b.searchBenchmark.elements.toLocaleString()} items)</h3>
                        <span class="badge badge-primary">Module VII</span>
                    </div>
                    <div class="speedup-callout">${b.searchBenchmark.speedupFactor.toFixed(1)}x FASTER</div>
                    <p style="font-size: 13px; color: var(--text-secondary); margin-bottom: 12px;">Binary Search vs Linear Search</p>
                    <table class="data-table">
                        <tr>
                            <td><strong>Linear Search O(N)</strong></td>
                            <td>${b.searchBenchmark.linearSearch.timeMicroseconds.toFixed(2)} µs</td>
                            <td>Index ${b.searchBenchmark.linearSearch.foundIndex}</td>
                        </tr>
                        <tr>
                            <td><strong class="text-green">Binary Search O(log N)</strong></td>
                            <td class="text-green">${b.searchBenchmark.binarySearch.timeMicroseconds.toFixed(2)} µs</td>
                            <td>Index ${b.searchBenchmark.binarySearch.foundIndex}</td>
                        </tr>
                    </table>
                </div>

                <!-- Sort Benchmark Card -->
                <div class="card">
                    <div class="card-header">
                        <h3>⚡ Sort Benchmark (${b.sortBenchmark.elements.toLocaleString()} items)</h3>
                        <span class="badge badge-primary">Module VII</span>
                    </div>
                    <div class="speedup-callout">${b.sortBenchmark.speedupFactor.toFixed(1)}x FASTER</div>
                    <p style="font-size: 13px; color: var(--text-secondary); margin-bottom: 12px;">std::sort (Introsort) vs Bubble Sort</p>
                    <table class="data-table">
                        <tr>
                            <td><strong>Bubble Sort O(N²)</strong></td>
                            <td>${b.sortBenchmark.bubbleSort.timeMilliseconds.toFixed(3)} ms</td>
                            <td>O(1) space</td>
                        </tr>
                        <tr>
                            <td><strong class="text-green">Introsort O(N log N)</strong></td>
                            <td class="text-green">${b.sortBenchmark.introsort.timeMilliseconds.toFixed(3)} ms</td>
                            <td>O(log N) space</td>
                        </tr>
                    </table>
                </div>
            </div>
        `;
        showToast('Benchmark run completed successfully!');
    }
}

async function runCompareDS() {
    const area = document.getElementById('benchmarkResultsArea');
    area.innerHTML = '<div class="empty-state">⚖️ Executing 50,000 pushes/pops on Hand-crafted Array vs STL structures...</div>';

    const resp = await callApi('/compare-ds?iters=50000');
    if (resp.success && resp.data) {
        const c = resp.data;
        area.innerHTML = `
            <div class="benchmark-card-grid">
                <div class="card">
                    <div class="card-header">
                        <h3>📚 Stack Comparison (${c.testIterations.toLocaleString()} operations)</h3>
                        <span class="badge badge-success">Module VIII vs X</span>
                    </div>
                    <table class="data-table" style="margin-top: 10px;">
                        <thead>
                            <tr>
                                <th>Implementation</th>
                                <th>Push (µs)</th>
                                <th>Pop (µs)</th>
                                <th>Memory Allocation</th>
                            </tr>
                        </thead>
                        <tbody>
                            <tr>
                                <td><strong class="text-green">Hand-crafted ArrayStack</strong></td>
                                <td class="text-green">${c.stackComparison.customArrayStackPushMicroseconds.toFixed(2)} µs</td>
                                <td class="text-green">${c.stackComparison.customArrayStackPopMicroseconds.toFixed(2)} µs</td>
                                <td>Contiguous array (Zero heap allocations, cache optimal)</td>
                            </tr>
                            <tr>
                                <td><strong>STL std::stack</strong></td>
                                <td>${c.stackComparison.stlStackPushMicroseconds.toFixed(2)} µs</td>
                                <td>${c.stackComparison.stlStackPopMicroseconds.toFixed(2)} µs</td>
                                <td>Dynamic chunked nodes (std::deque adapter)</td>
                            </tr>
                        </tbody>
                    </table>
                </div>

                <div class="card">
                    <div class="card-header">
                        <h3>🔄 Queue Comparison (${c.testIterations.toLocaleString()} operations)</h3>
                        <span class="badge badge-success">Module IX vs X</span>
                    </div>
                    <table class="data-table" style="margin-top: 10px;">
                        <thead>
                            <tr>
                                <th>Implementation</th>
                                <th>Enqueue (µs)</th>
                                <th>Dequeue (µs)</th>
                                <th>Memory Strategy</th>
                            </tr>
                        </thead>
                        <tbody>
                            <tr>
                                <td><strong class="text-green">Hand-crafted CircularQueue</strong></td>
                                <td class="text-green">${c.queueComparison.customCircularQueueEnqueueMicroseconds.toFixed(2)} µs</td>
                                <td class="text-green">${c.queueComparison.customCircularQueueDequeueMicroseconds.toFixed(2)} µs</td>
                                <td>Bounded ring buffer, modulo index arithmetic</td>
                            </tr>
                            <tr>
                                <td><strong>STL std::queue</strong></td>
                                <td>${c.queueComparison.stlQueuePushMicroseconds.toFixed(2)} µs</td>
                                <td>${c.queueComparison.stlQueuePopMicroseconds.toFixed(2)} µs</td>
                                <td>Segmented node buffers (std::deque adapter)</td>
                            </tr>
                        </tbody>
                    </table>
                </div>
            </div>
        `;
        showToast('Array DS vs STL comparison completed!');
    }
}

// Initial Boot
window.addEventListener('DOMContentLoaded', () => {
    loadRestaurants();
    loadMenu();
    refreshCartView();
});
