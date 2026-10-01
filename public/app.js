// ============================================================================
// FoodRush - Modern Storefront & Administration Client
// Dual-Surface Architecture: Customer App / Admin vs Hidden Viva Examiner Mode
// ============================================================================

const API_BASE = '/api';

// Application State
let appState = {
    restaurants: [],
    menu: [],
    activeRestaurantId: 0,
    activeCuisine: 'All',
    activeDiet: 'all',
    cart: { itemCount: 0, items: [], total: 0, undoStackDepth: 0 },
    activeOrder: null,
    searchQuery: '',
    searchDebounceTimer: null,
    vivaActive: false,
    zoneNames: ["Central", "North", "South", "East", "West"]
};

// ============================================================================
// NAVIGATION & ROUTING
// ============================================================================

function navigateTo(viewId) {
    document.querySelectorAll('.app-view').forEach(el => el.classList.remove('active'));
    document.querySelectorAll('.nav-link').forEach(el => el.classList.remove('active'));

    const targetView = document.getElementById(
        viewId === 'admin' ? 'viewAdmin' : (viewId === 'tracker' ? 'viewTracker' : 'viewStorefront')
    );
    if (targetView) targetView.classList.add('active');

    const navBtn = document.getElementById(
        viewId === 'admin' ? 'navAdmin' : (viewId === 'tracker' ? 'navTracker' : 'navStorefront')
    );
    if (navBtn) navBtn.classList.add('active');

    if (viewId === 'admin') {
        refreshAdminDashboard();
    } else if (viewId === 'tracker' && !appState.activeOrder) {
        lookupOrder(); // Fetch latest order if available
    }

    window.scrollTo({ top: 0, behavior: 'smooth' });
}

window.addEventListener('hashchange', () => {
    const hash = window.location.hash.replace('#', '');
    if (hash === 'admin') navigateTo('admin');
    else if (hash === 'tracker') navigateTo('tracker');
    else navigateTo('storefront');
});

// ============================================================================
// API CALLER WITH ERROR HANDLING & VIVA TRACKING
// ============================================================================

async function callApi(endpoint, method = 'GET', body = null) {
    try {
        const options = { method, headers: { 'Content-Type': 'application/json' } };
        if (body) options.body = JSON.stringify(body);

        const res = await fetch(`${API_BASE}${endpoint}`, options);
        const json = await res.json();

        // Update Viva mode badge if active
        const modules = json.modules || json.handled_by;
        if (modules && modules.length > 0) {
            updateVivaBadge(modules);
        }

        return json;
    } catch (err) {
        console.error(`API Error on ${endpoint}:`, err);
        return { success: false, message: err.message };
    }
}

// ============================================================================
// STOREFRONT: RESTAURANTS & MENU
// ============================================================================

async function loadStorefrontData() {
    const [restRes, menuRes] = await Promise.all([
        callApi('/restaurants'),
        callApi('/menu')
    ]);

    if (restRes.success) {
        appState.restaurants = restRes.data || [];
        renderRestaurantsGrid(appState.restaurants);
    }

    if (menuRes.success) {
        appState.menu = menuRes.data || [];
        renderDishesGrid(appState.menu);
    }
}

function renderRestaurantsGrid(restaurants) {
    const container = document.getElementById('restaurantsGrid');
    if (!container) return;

    // Palette of tasteful background colors for cuisine artwork
    const cuisineArts = {
        "Biryani & Mughlai": { bg: "#FBF2E9", color: "#D9531E", icon: "🍛" },
        "South Indian": { bg: "#EDF5F0", color: "#2F7D5B", icon: "🥞" },
        "Artisan Italian": { bg: "#F9ECE8", color: "#B83A20", icon: "🍕" },
        "Japanese": { bg: "#F4EFF6", color: "#7B4F8D", icon: "🍜" },
        "American Gourmet": { bg: "#FBF5E6", color: "#A87216", icon: "🍔" },
        "Desserts & Bakery": { bg: "#FBF0F4", color: "#B34A7B", icon: "🍰" }
    };

    container.innerHTML = restaurants.map(r => {
        const art = cuisineArts[r.cuisine] || { bg: "#F4F1EC", color: "#6B645B", icon: "🍽️" };
        const isActive = appState.activeRestaurantId === r.id;

        return `
            <div class="restaurant-card ${isActive ? 'active' : ''}" onclick="selectRestaurant(${r.id})">
                <div class="rest-artwork" style="background-color: ${art.bg};">
                    <span style="font-size: 40px;">${art.icon}</span>
                    <span class="rest-cuisine-badge">${r.cuisine}</span>
                </div>
                <div class="rest-info">
                    <div class="rest-header-row">
                        <h3 class="rest-name">${r.name}</h3>
                        <span class="rest-rating-pill">★ ${r.rating.toFixed(1)}</span>
                    </div>
                    <div class="rest-stats-row">
                        <span>${r.etaMinutes} mins</span>
                        <span class="dot-separator"></span>
                        <span>Zone ${r.zoneId} (${appState.zoneNames[r.zoneId]})</span>
                        <span class="dot-separator"></span>
                        <span>Min ₹${r.minOrder.toFixed(0)}</span>
                    </div>
                </div>
            </div>
        `;
    }).join('');
}

function selectRestaurant(restId) {
    if (appState.activeRestaurantId === restId) {
        appState.activeRestaurantId = 0; // Toggle off filter
        document.getElementById('menuRestaurantName').textContent = "All Dishes";
        document.getElementById('menuRestaurantMeta').textContent = "Showing freshly prepared specialities";
    } else {
        appState.activeRestaurantId = restId;
        const rest = appState.restaurants.find(r => r.id === restId);
        if (rest) {
            document.getElementById('menuRestaurantName').textContent = rest.name;
            document.getElementById('menuRestaurantMeta').textContent = `${rest.cuisine} · Minimum order ₹${rest.minOrder.toFixed(0)}`;
        }
    }

    renderRestaurantsGrid(appState.restaurants);
    applyDishesFilter();

    const menuEl = document.getElementById('menuSection');
    if (menuEl) menuEl.scrollIntoView({ behavior: 'smooth' });
}

function selectCuisineFilter(cuisine) {
    appState.activeCuisine = cuisine;
    document.querySelectorAll('.cuisine-chip').forEach(btn => {
        btn.classList.toggle('active', btn.textContent.trim().includes(cuisine) || (cuisine === 'All' && btn.textContent.includes('All')));
    });

    applyDishesFilter();
}

function filterMenuByDiet(diet) {
    appState.activeDiet = diet;
    document.querySelectorAll('.filter-pill').forEach(btn => {
        btn.classList.toggle('active', btn.getAttribute('onclick')?.includes(diet));
    });

    applyDishesFilter();
}

function applyDishesFilter() {
    let filtered = [...appState.menu];

    // Filter by restaurant
    if (appState.activeRestaurantId > 0) {
        filtered = filtered.filter(d => d.restaurantId === appState.activeRestaurantId);
    }

    // Filter by cuisine
    if (appState.activeCuisine !== 'All') {
        const matchingRestIds = appState.restaurants
            .filter(r => r.cuisine.toLowerCase().includes(appState.activeCuisine.toLowerCase()))
            .map(r => r.id);
        filtered = filtered.filter(d => matchingRestIds.includes(d.restaurantId));
    }

    // Filter by diet
    if (appState.activeDiet === 'veg') {
        filtered = filtered.filter(d => d.isVeg);
    } else if (appState.activeDiet === 'nonveg') {
        filtered = filtered.filter(d => !d.isVeg);
    } else if (appState.activeDiet === 'special') {
        filtered = filtered.filter(d => d.isChefSpecial);
    }

    renderDishesGrid(filtered);
}

function renderDishesGrid(dishes) {
    const container = document.getElementById('dishesGrid');
    if (!container) return;

    if (dishes.length === 0) {
        container.innerHTML = `<div class="empty-state-card" style="grid-column: 1 / -1;"><p>No dishes match your active filter.</p></div>`;
        return;
    }

    container.innerHTML = dishes.map(dish => {
        const inCartItem = appState.cart.items?.find(c => c.itemId === dish.id);
        const qty = inCartItem ? inCartItem.quantity : 0;

        return `
            <div class="dish-card">
                <div class="dish-top">
                    <div class="dish-indicator-row">
                        <span class="${dish.isVeg ? 'mark-veg' : 'mark-nonveg'}" title="${dish.isVeg ? 'Vegetarian' : 'Non-Vegetarian'}"></span>
                        ${dish.isChefSpecial ? '<span class="tag-badge tag-chef">Chef Special</span>' : ''}
                        ${dish.isSpicy ? '<span class="tag-badge tag-spicy">Spicy</span>' : ''}
                        ${dish.isGlutenFree ? '<span class="tag-badge tag-gf">Gluten Free</span>' : ''}
                    </div>
                    <h4 class="dish-name">${dish.name}</h4>
                    <span class="dish-details">${dish.category} · ${dish.calories} kcal</span>
                </div>
                <div class="dish-bottom">
                    <span class="dish-price">₹${dish.price.toFixed(2)}</span>
                    <div>
                        ${qty === 0 ? `
                            <button class="btn-add-item" onclick="addToCart(${dish.id}, 1)">+ Add</button>
                        ` : `
                            <div class="qty-stepper">
                                <button class="stepper-btn" onclick="addToCart(${dish.id}, -1)">−</button>
                                <span class="stepper-qty">${qty}</span>
                                <button class="stepper-btn" onclick="addToCart(${dish.id}, 1)">+</button>
                            </div>
                        `}
                    </div>
                </div>
            </div>
        `;
    }).join('');
}

// ============================================================================
// SEARCH LOGIC & DID YOU MEAN
// ============================================================================

function handleSearchInput(e) {
    clearTimeout(appState.searchDebounceTimer);
    const query = e.target.value.trim();
    appState.searchQuery = query;

    const clearBtn = document.getElementById('searchClearBtn');
    if (clearBtn) clearBtn.style.display = query.length > 0 ? 'block' : 'none';

    if (query.length === 0) {
        clearSearch();
        return;
    }

    // Debounce search by 180ms
    appState.searchDebounceTimer = setTimeout(async () => {
        const resp = await callApi(`/search?q=${encodeURIComponent(query)}`);
        const resultsSection = document.getElementById('searchResultsSection');
        const resultsGrid = document.getElementById('searchResultsGrid');
        const heading = document.getElementById('searchResultsHeading');
        const didYouMeanBar = document.getElementById('didYouMeanBar');
        const didYouMeanChip = document.getElementById('didYouMeanChip');

        if (resp.success && resp.data) {
            resultsSection.style.display = 'block';
            const matches = resp.data.matches || resp.data;
            heading.textContent = `Search results for "${query}" (${matches.length})`;

            if (resp.data.didYouMean && resp.data.didYouMean.length > 0) {
                didYouMeanBar.style.display = 'flex';
                didYouMeanChip.textContent = resp.data.didYouMean;
            } else {
                didYouMeanBar.style.display = 'none';
            }

            if (matches.length === 0) {
                resultsGrid.innerHTML = `
                    <div class="empty-state-card" style="grid-column: 1 / -1;">
                        <p>No dishes found matching "${query}".</p>
                    </div>
                `;
            } else {
                resultsGrid.innerHTML = matches.map(dish => `
                    <div class="dish-card">
                        <div class="dish-top">
                            <div class="dish-indicator-row">
                                <span class="${dish.isVeg ? 'mark-veg' : 'mark-nonveg'}"></span>
                                ${dish.isChefSpecial ? '<span class="tag-badge tag-chef">Special</span>' : ''}
                                ${dish.isSpicy ? '<span class="tag-badge tag-spicy">Spicy</span>' : ''}
                            </div>
                            <h4 class="dish-name">${dish.name}</h4>
                            <span class="dish-details">${dish.category} · ${dish.calories} kcal</span>
                        </div>
                        <div class="dish-bottom">
                            <span class="dish-price">₹${dish.price.toFixed(2)}</span>
                            <button class="btn-add-item" onclick="addToCart(${dish.id}, 1)">+ Add</button>
                        </div>
                    </div>
                `).join('');
            }
        }
    }, 180);
}

function clearSearch() {
    const input = document.getElementById('mainSearchInput');
    if (input) input.value = '';
    document.getElementById('searchClearBtn').style.display = 'none';
    document.getElementById('searchResultsSection').style.display = 'none';
    document.getElementById('didYouMeanBar').style.display = 'none';
}

function applySuggestion() {
    const chip = document.getElementById('didYouMeanChip');
    const input = document.getElementById('mainSearchInput');
    if (chip && input) {
        input.value = chip.textContent;
        handleSearchInput({ target: input });
    }
}

// ============================================================================
// CART & UNDO LOGIC
// ============================================================================

function toggleCartDrawer(open) {
    const drawer = document.getElementById('cartDrawer');
    const overlay = document.getElementById('cartDrawerOverlay');
    if (open) {
        drawer.classList.add('active');
        overlay.classList.add('active');
        refreshCartView();
    } else {
        drawer.classList.remove('active');
        overlay.classList.remove('active');
    }
}

async function addToCart(itemId, quantityDelta) {
    const inCart = appState.cart.items?.find(c => c.itemId === itemId);
    const currentQty = inCart ? inCart.quantity : 0;
    const targetQty = currentQty + quantityDelta;

    let resp;
    if (targetQty <= 0) {
        resp = await callApi('/cart/remove', 'POST', { itemId });
    } else if (currentQty === 0) {
        resp = await callApi('/cart/add', 'POST', { itemId, quantity: quantityDelta });
    } else {
        // Delta adjustment
        resp = await callApi('/cart/add', 'POST', { itemId, quantity: quantityDelta });
    }

    if (resp.success) {
        appState.cart = resp.data;
        updateCartBadgeCount();
        renderDishesGrid(appState.menu);
        renderCartDrawer();
        showUndoToast(resp.message || "Your order was updated");
    }
}

async function cartUndo() {
    const resp = await callApi('/cart/undo', 'POST');
    if (resp.success) {
        appState.cart = resp.data;
        updateCartBadgeCount();
        renderDishesGrid(appState.menu);
        renderCartDrawer();
        showUndoToast(`Reverted: ${resp.message}`);
    }
}

async function cartClear() {
    const resp = await callApi('/cart/clear', 'POST');
    if (resp.success) {
        appState.cart = resp.data;
        updateCartBadgeCount();
        renderDishesGrid(appState.menu);
        renderCartDrawer();
    }
}

async function refreshCartView() {
    const zone = document.getElementById('cartZoneSelect')?.value || 0;
    const express = document.getElementById('cartExpressCheckbox')?.checked ? 1 : 0;

    const resp = await callApi(`/cart?zone=${zone}&express=${express}`);
    if (resp.success) {
        appState.cart = resp.data;
        updateCartBadgeCount();
        renderCartDrawer();
    }
}

function updateCartBadgeCount() {
    const count = appState.cart.itemCount || 0;
    const badge = document.getElementById('headerCartCount');
    if (badge) badge.textContent = count;
}

function renderCartDrawer() {
    const body = document.getElementById('cartDrawerBody');
    const footer = document.getElementById('cartDrawerFooter');
    const countChip = document.getElementById('drawerItemCount');

    countChip.textContent = `${appState.cart.itemCount || 0} items`;

    if (!appState.cart.items || appState.cart.items.length === 0) {
        body.innerHTML = `
            <div class="drawer-empty-state">
                <svg width="44" height="44" viewBox="0 0 24 24" fill="none" stroke="#6B645B" stroke-width="1.5"><circle cx="9" cy="21" r="1"/><circle cx="20" cy="21" r="1"/><path d="M1 1h4l2.68 13.39a2 2 0 0 0 2 1.61h9.72a2 2 0 0 0 2-1.61L23 6H6"/></svg>
                <p style="font-weight:600; color:var(--text-main);">Your bag is empty.</p>
                <span style="font-size:13px; color:var(--text-muted);">Explore our menus and add your favourite dishes.</span>
            </div>
        `;
        footer.style.display = 'none';
        return;
    }

    footer.style.display = 'flex';

    body.innerHTML = appState.cart.items.map(item => `
        <div class="cart-item-row">
            <div class="cart-item-info">
                <span class="cart-item-name">${item.name}</span>
                <span class="cart-item-unit-price">₹${item.price.toFixed(2)} each</span>
            </div>
            <div class="cart-item-controls">
                <div class="qty-stepper">
                    <button class="stepper-btn" onclick="addToCart(${item.itemId}, -1)">−</button>
                    <span class="stepper-qty">${item.quantity}</span>
                    <button class="stepper-btn" onclick="addToCart(${item.itemId}, 1)">+</button>
                </div>
                <span class="cart-item-total">₹${item.lineTotal.toFixed(2)}</span>
            </div>
        </div>
    `).join('');

    // Summary numbers
    document.getElementById('billSubtotal').textContent = `₹${(appState.cart.subtotal || 0).toFixed(2)}`;
    document.getElementById('billTax').textContent = `₹${(appState.cart.tax || 0).toFixed(2)}`;
    document.getElementById('billDelivery').textContent = `₹${(appState.cart.deliveryFee || 0).toFixed(2)}`;
    document.getElementById('billTotal').textContent = `₹${(appState.cart.total || 0).toFixed(2)}`;
    document.getElementById('btnPayAmount').textContent = `₹${(appState.cart.total || 0).toFixed(2)}`;

    const discountRow = document.getElementById('billDiscountRow');
    const discountVal = document.getElementById('billDiscount');
    if (appState.cart.discountAmount > 0) {
        discountRow.style.display = 'flex';
        discountVal.textContent = `-₹${appState.cart.discountAmount.toFixed(2)}`;
    } else {
        discountRow.style.display = 'none';
    }
}

async function applyCoupon() {
    const input = document.getElementById('drawerCouponInput');
    const code = input ? input.value.trim() : '';
    if (!code) return;

    const resp = await callApi('/coupon', 'POST', { code });
    if (resp.success) {
        showUndoToast(resp.message);
        refreshCartView();
    } else {
        showUndoToast(resp.message);
    }
}

function showUndoToast(msg) {
    const toast = document.getElementById('undoToast');
    const msgEl = document.getElementById('undoToastMsg');
    if (!toast || !msgEl) return;

    msgEl.textContent = msg;
    toast.classList.add('show');
    clearTimeout(toast._timer);
    toast._timer = setTimeout(() => toast.classList.remove('show'), 4000);
}

// ============================================================================
// CHECKOUT & LIVE ORDER TRACKING
// ============================================================================

async function submitCheckout() {
    if (!appState.cart.items || appState.cart.items.length === 0) return;

    const name = document.getElementById('checkoutCustName')?.value.trim() || 'Aarav Sharma';
    const street = document.getElementById('checkoutCustStreet')?.value.trim() || '42 Residency Road';
    const zoneId = parseInt(document.getElementById('cartZoneSelect')?.value || 0);
    const isExpress = document.getElementById('cartExpressCheckbox')?.checked || false;

    const resp = await callApi('/checkout', 'POST', {
        customerName: name,
        street: street,
        zoneId: zoneId,
        isExpress: isExpress
    });

    if (resp.success && resp.data) {
        appState.activeOrder = resp.data;
        toggleCartDrawer(false);
        refreshCartView();
        navigateTo('tracker');
        renderActiveOrderTracking(appState.activeOrder);
    }
}

async function lookupOrder() {
    const input = document.getElementById('trackCodeInput');
    const code = input ? input.value.trim() : '';

    const resp = await callApi(code ? `/track?code=${encodeURIComponent(code)}` : '/track');
    if (resp.success && resp.data && resp.data.order) {
        appState.activeOrder = resp.data.order;
        renderActiveOrderTracking(appState.activeOrder, resp.data.trackingCodeVerified);
    }
}

async function simulateNextStage() {
    const resp = await callApi('/orders/simulate', 'POST', {
        orderId: appState.activeOrder ? appState.activeOrder.orderId : null
    });

    if (resp.success && resp.data) {
        appState.activeOrder = resp.data;
        renderActiveOrderTracking(appState.activeOrder, true);
        showUndoToast(`Order stage advanced: ${resp.data.status}`);
    }
}

function renderActiveOrderTracking(order, verified = true) {
    const container = document.getElementById('trackerDetailsArea');
    if (!container || !order) return;

    const stages = ["PLACED", "PREPARING", "OUT_FOR_DELIVERY", "DELIVERED"];
    const currentIndex = stages.indexOf(order.status);

    const statusBadgeClass = order.status === 'PLACED' ? 'badge-placed'
        : (order.status === 'PREPARING' ? 'badge-prep'
        : (order.status === 'OUT_FOR_DELIVERY' ? 'badge-transit' : 'badge-delivered'));

    const readableStatus = order.status === 'PLACED' ? 'Order Confirmed'
        : (order.status === 'PREPARING' ? 'Preparing in Kitchen'
        : (order.status === 'OUT_FOR_DELIVERY' ? 'On Its Way' : 'Delivered'));

    container.innerHTML = `
        <div class="order-tracking-card">
            <div class="tracking-header-row">
                <div>
                    <h2 class="tracking-order-num">Order #${order.orderId}</h2>
                    <div class="tracking-sub-meta">
                        Tracking Code: <strong style="font-family:var(--font-mono);">${order.trackingCode}</strong>
                        ${verified ? ' · <span style="color:var(--success); font-weight:600;">Code verified</span>' : ''}
                    </div>
                </div>
                <span class="status-badge-live ${statusBadgeClass}">${readableStatus}</span>
            </div>

            <!-- Stepper -->
            <div class="tracker-stepper">
                <div class="step-node ${currentIndex >= 0 ? (currentIndex > 0 ? 'completed' : 'current') : ''}">
                    <div class="step-circle">${currentIndex > 0 ? '✓' : '1'}</div>
                    <span class="step-label">Placed</span>
                </div>
                <div class="step-node ${currentIndex >= 1 ? (currentIndex > 1 ? 'completed' : 'current') : ''}">
                    <div class="step-circle">${currentIndex > 1 ? '✓' : '2'}</div>
                    <span class="step-label">Kitchen</span>
                </div>
                <div class="step-node ${currentIndex >= 2 ? (currentIndex > 2 ? 'completed' : 'current') : ''}">
                    <div class="step-circle">${currentIndex > 2 ? '✓' : '3'}</div>
                    <span class="step-label">Courier</span>
                </div>
                <div class="step-node ${currentIndex >= 3 ? 'completed' : ''}">
                    <div class="step-circle">${currentIndex >= 3 ? '✓' : '4'}</div>
                    <span class="step-label">Arrived</span>
                </div>
            </div>

            <!-- Live Status Highlights -->
            <div class="tracking-info-grid">
                <div class="info-box">
                    <span class="info-label">Queue Position</span>
                    <span class="info-value">
                        ${order.status === 'PLACED' && order.queuePosition > 0
                            ? `You are #${order.queuePosition} in line`
                            : (order.status === 'PREPARING' ? 'Chef is cooking' : (order.status === 'OUT_FOR_DELIVERY' ? 'Out for delivery' : 'Completed'))}
                    </span>
                </div>
                <div class="info-box">
                    <span class="info-label">Estimated Time</span>
                    <span class="info-value">${order.estimatedMinutes > 0 ? `${order.estimatedMinutes} minutes` : 'Delivered'}</span>
                </div>
                <div class="info-box">
                    <span class="info-label">Delivery Partner</span>
                    <span class="info-value">${order.riderName}</span>
                </div>
                <div class="info-box">
                    <span class="info-label">Delivering To</span>
                    <span class="info-value">${order.address?.street || 'Customer Address'}, Zone ${order.address?.zoneId || 0}</span>
                </div>
            </div>

            <!-- Simulation Controls for Demos -->
            <div class="simulation-bar">
                <span style="font-size: 13px; color: var(--text-muted);">Demonstration Control</span>
                <button class="secondary-btn" onclick="simulateNextStage()">Simulate Next Stage ➔</button>
            </div>
        </div>
    `;
}

// ============================================================================
// ADMIN DASHBOARD
// ============================================================================

async function refreshAdminDashboard() {
    const [matrixRes, ordersRes, ridersRes] = await Promise.all([
        callApi('/sales-matrix'),
        callApi('/orders'),
        callApi('/riders')
    ]);

    if (matrixRes.success && matrixRes.data) {
        renderSalesMatrix(matrixRes.data);
    }

    if (ordersRes.success && ordersRes.data) {
        renderAdminOrders(ordersRes.data);
    }

    if (ridersRes.success && ridersRes.data) {
        renderAdminRiders(ridersRes.data);
    }
}

function renderSalesMatrix(matrix) {
    const metricsContainer = document.getElementById('adminMetricsGrid');
    if (metricsContainer) {
        metricsContainer.innerHTML = `
            <div class="metric-tile">
                <div class="metric-tile-label">Total Platform Sales</div>
                <div class="metric-tile-val">₹${matrix.grandTotal.toLocaleString('en-IN', { maximumFractionDigits: 0 })}</div>
                <div class="metric-tile-sub">Full 7-day revenue</div>
            </div>
            <div class="metric-tile">
                <div class="metric-tile-label">Busiest Day</div>
                <div class="metric-tile-val">${matrix.days[matrix.busiestDay]}</div>
                <div class="metric-tile-sub">Peak single slot: ₹${matrix.peakSalesAmount.toFixed(0)}</div>
            </div>
            <div class="metric-tile">
                <div class="metric-tile-label">Active Kitchens</div>
                <div class="metric-tile-val">${appState.restaurants.length}</div>
                <div class="metric-tile-sub">Across 5 delivery zones</div>
            </div>
        `;
    }

    const tableContainer = document.getElementById('salesMatrixContainer');
    if (!tableContainer) return;

    const restNames = appState.restaurants.map(r => r.name);

    let html = `
        <table class="data-table">
            <thead>
                <tr>
                    <th>Kitchen Partner</th>
                    ${matrix.days.map(d => `<th>${d}</th>`).join('')}
                    <th>Weekly Total</th>
                </tr>
            </thead>
            <tbody>
    `;

    for (let r = 0; r < matrix.sales.length; ++r) {
        html += `<tr><td><strong>${restNames[r] || `Kitchen ${r+1}`}</strong></td>`;
        for (let d = 0; d < 7; ++d) {
            const isPeak = (r === matrix.busiestRestaurant && d === matrix.busiestDay);
            html += `<td class="${isPeak ? 'peak-revenue-cell' : ''}">₹${matrix.sales[r][d].toFixed(0)}</td>`;
        }
        html += `<td><strong>₹${matrix.restaurantWeeklyTotals[r].toFixed(0)}</strong></td></tr>`;
    }

    // Daily totals row
    html += `
        <tr style="background-color: var(--surface-soft); font-weight: 700;">
            <td>Daily Total</td>
            ${matrix.dailyTotals.map(tot => `<td>₹${tot.toFixed(0)}</td>`).join('')}
            <td>₹${matrix.grandTotal.toFixed(0)}</td>
        </tr>
        </tbody></table>
    `;

    tableContainer.innerHTML = html;

    // Distance Matrix Table
    const distContainer = document.getElementById('distanceMatrixContainer');
    if (distContainer) {
        let distHtml = `
            <table class="data-table">
                <thead>
                    <tr>
                        <th>Origin / Destination</th>
                        ${matrix.zones.map(z => `<th>${z}</th>`).join('')}
                    </tr>
                </thead>
                <tbody>
        `;
        for (let i = 0; i < matrix.zones.length; ++i) {
            distHtml += `<tr><td><strong>${matrix.zones[i]}</strong></td>`;
            for (let j = 0; j < matrix.zones.length; ++j) {
                distHtml += `<td>${matrix.distanceMatrix[i][j].toFixed(1)} km</td>`;
            }
            distHtml += `</tr>`;
        }
        distHtml += `</tbody></table>`;
        distContainer.innerHTML = distHtml;
    }
}

function renderAdminOrders(ordersData) {
    const container = document.getElementById('adminOrdersTableContainer');
    if (!container) return;

    const orders = ordersData.orders || [];
    if (orders.length === 0) {
        container.innerHTML = `<p style="padding: 20px; color: var(--text-muted);">No active orders.</p>`;
        return;
    }

    container.innerHTML = `
        <table class="data-table">
            <thead>
                <tr>
                    <th>Order #</th>
                    <th>Customer</th>
                    <th>Kitchen</th>
                    <th>Type</th>
                    <th>Amount</th>
                    <th>Status</th>
                    <th>Action</th>
                </tr>
            </thead>
            <tbody>
                ${orders.map(o => `
                    <tr>
                        <td><strong>#${o.orderId}</strong></td>
                        <td>${o.customerName}</td>
                        <td>${o.restaurantName}</td>
                        <td>${o.isExpress ? '<span style="color:var(--accent); font-weight:700;">⚡ Express</span>' : 'Standard'}</td>
                        <td>₹${o.totalAmount.toFixed(2)}</td>
                        <td><span class="status-badge-live badge-${o.status.toLowerCase().replace(/_/g, '')}">${o.status}</span></td>
                        <td>
                            ${o.status === 'PLACED' ? `<button class="btn-add-item" onclick="adminCookOrder(${o.orderId})">Cook</button>` : ''}
                            ${o.status === 'PREPARING' ? `<button class="btn-add-item" onclick="adminAssignRider(${o.orderId})">Dispatch</button>` : ''}
                            ${o.status === 'OUT_FOR_DELIVERY' ? `<button class="btn-add-item" onclick="adminCompleteOrder(${o.orderId})">Deliver</button>` : ''}
                            ${o.status === 'DELIVERED' ? '<span style="color:var(--success); font-size:12px;">Delivered</span>' : ''}
                        </td>
                    </tr>
                `).join('')}
            </tbody>
        </table>
    `;
}

function renderAdminRiders(riders) {
    const container = document.getElementById('adminRidersGrid');
    if (!container) return;

    container.innerHTML = riders.map(r => `
        <div class="rider-admin-card">
            <span class="rider-name">${r.name}</span>
            <span class="rider-detail">🛵 ${r.vehicle}</span>
            <span class="rider-detail">Zone ${r.currentZone} (${appState.zoneNames[r.currentZone]}) · ${r.totalDeliveries} trips</span>
            <span style="font-size: 11px; font-weight: 700; color: ${r.isAvailable ? 'var(--success)' : 'var(--accent)'}; margin-top: 4px;">
                ${r.isAvailable ? '● Available' : '● On Delivery'}
            </span>
        </div>
    `).join('');
}

async function adminCookOrder(orderId) {
    await callApi('/orders/cook', 'POST', { orderId });
    refreshAdminDashboard();
}

async function adminCookNextOrder() {
    await callApi('/orders/cook', 'POST', {});
    refreshAdminDashboard();
}

async function adminAssignRider(orderId) {
    await callApi('/orders/assign-rider', 'POST', { orderId });
    refreshAdminDashboard();
}

async function adminCompleteOrder(orderId) {
    await callApi('/orders/complete', 'POST', { orderId });
    refreshAdminDashboard();
}

// ============================================================================
// VIVA EXAMINER MODE (TRIGGERED VIA KEY 'V' OR FOOTER LINK)
// ============================================================================

function toggleVivaMode(force) {
    if (typeof force === 'boolean') {
        appState.vivaActive = force;
    } else {
        appState.vivaActive = !appState.vivaActive;
    }

    const banner = document.getElementById('vivaTopBanner');
    if (banner) banner.style.display = appState.vivaActive ? 'block' : 'none';

    if (appState.vivaActive) {
        showUndoToast("Viva Examiner Mode Activated (Press V to toggle)");
    }
}

// Global keyboard shortcut: key 'v' or 'V' toggles Viva Mode
window.addEventListener('keydown', (e) => {
    // Only toggle if not actively typing in an input
    if (e.target.tagName !== 'INPUT' && e.target.tagName !== 'TEXTAREA') {
        if (e.key === 'v' || e.key === 'V') {
            toggleVivaMode();
        }
    }
});

function updateVivaBadge(modules) {
    const badge = document.getElementById('vivaHandledBadge');
    if (!badge || !modules || modules.length === 0) return;
    badge.textContent = `Handled by: ${modules.join(' + ')}`;
}

function openVivaModal(tab = 'inspector') {
    const modal = document.getElementById('vivaModalOverlay');
    if (modal) modal.style.display = 'flex';
    switchVivaTab(tab);
    if (tab === 'inspector') loadVivaInspectorData();
}

function closeVivaModal() {
    const modal = document.getElementById('vivaModalOverlay');
    if (modal) modal.style.display = 'none';
}

function switchVivaTab(tabId) {
    document.querySelectorAll('.viva-pane').forEach(el => el.classList.remove('active'));
    document.querySelectorAll('.viva-tab-btn').forEach(el => el.classList.remove('active'));

    const pane = document.getElementById(
        tabId === 'benchmarks' ? 'vivaPaneBenchmarks' : (tabId === 'map' ? 'vivaPaneMap' : 'vivaPaneInspector')
    );
    if (pane) pane.classList.add('active');

    const btn = document.getElementById(`vtab-${tabId}`);
    if (btn) btn.classList.add('active');

    if (tabId === 'inspector') loadVivaInspectorData();
}

async function loadVivaInspectorData() {
    const resp = await callApi('/inspect');
    if (resp.success && resp.data) {
        const d = resp.data;

        // Stack View
        document.getElementById('vivaStackCount').textContent = `Depth: ${d.stackInspector.size}/${d.stackInspector.capacity}`;
        const stackView = document.getElementById('vivaStackView');
        if (d.stackInspector.elements.length === 0) {
            stackView.innerHTML = `<div class="empty-viva">Stack is empty. Add or remove items to see frames.</div>`;
        } else {
            stackView.innerHTML = d.stackInspector.elements.map((frame, idx) => `
                <div class="viva-stack-frame">
                    <strong>[Top - ${idx}] ${frame.type}</strong> | ${frame.itemName} (ID: ${frame.itemId})
                    <span style="color:var(--text-subtle);">prev: ${frame.prevQty} ➔ new: ${frame.newQty}</span>
                </div>
            `).join('');
        }

        // Circular Queue View
        document.getElementById('vivaQueueCount').textContent = `Count: ${d.circularQueueInspector.count}/${d.circularQueueInspector.capacity}`;
        const queueView = document.getElementById('vivaQueueView');
        if (d.circularQueueInspector.elements.length === 0) {
            queueView.innerHTML = `<div class="empty-viva">Queue is empty. Place an order to see buffer slots.</div>`;
        } else {
            queueView.innerHTML = d.circularQueueInspector.elements.map((ordId, offset) => `
                <div class="viva-queue-slot ${offset === 0 ? 'front-slot' : ''}">
                    ${offset === 0 ? '👉 FRONT' : 'SLOT'}: Order #${ordId}
                </div>
            `).join('');
        }

        // Recently Viewed View
        document.getElementById('vivaRecentCount').textContent = `${d.recentlyViewedStack.size} items`;
        const recentView = document.getElementById('vivaRecentView');
        if (d.recentlyViewedStack.items.length === 0) {
            recentView.innerHTML = `<div class="empty-viva">No items viewed yet.</div>`;
        } else {
            recentView.innerHTML = `
                <div style="display:flex; gap:6px; flex-wrap:wrap;">
                    ${d.recentlyViewedStack.items.map(dish => `
                        <span class="viva-chip">#${dish.id}: ${dish.name}</span>
                    `).join('')}
                </div>
            `;
        }

        // STL Overview
        document.getElementById('vivaStlView').innerHTML = `
            <div>• std::map&lt;string, double&gt; (Coupons Registered): <strong>${d.stlOverview.couponsInMap}</strong></div>
            <div>• std::set&lt;string&gt; (Unique Cuisines Registered): <strong>${d.stlOverview.cuisinesInSet}</strong></div>
            <div>• std::vector&lt;std::pair&lt;int, double&gt;&gt; (Promo Pairs): <strong>${d.stlOverview.dailySpecialPairs}</strong></div>
        `;
    }
}

async function runVivaBenchmark() {
    const area = document.getElementById('vivaBenchmarkOutput');
    area.innerHTML = `<p style="color:var(--accent);">Executing high-resolution C++ benchmark on 30,000 search elements and 2,500 sort elements...</p>`;

    const resp = await callApi('/benchmark');
    if (resp.success && resp.data) {
        const b = resp.data;
        area.innerHTML = `
            <div style="display:grid; grid-template-columns:1fr 1fr; gap:16px;">
                <div class="viva-card">
                    <h4>Search Benchmark (${b.searchBenchmark.elements.toLocaleString()} items)</h4>
                    <p style="font-size:24px; font-weight:700; color:var(--success); margin:8px 0;">
                        ${b.searchBenchmark.speedupFactor.toFixed(1)}x Faster
                    </p>
                    <table class="viva-table">
                        <tr><td>Linear Search O(N)</td><td>${b.searchBenchmark.linearSearch.timeMicroseconds.toFixed(2)} µs</td></tr>
                        <tr><td>Binary Search O(log N)</td><td>${b.searchBenchmark.binarySearch.timeMicroseconds.toFixed(2)} µs</td></tr>
                    </table>
                </div>
                <div class="viva-card">
                    <h4>Sort Benchmark (${b.sortBenchmark.elements.toLocaleString()} items)</h4>
                    <p style="font-size:24px; font-weight:700; color:var(--success); margin:8px 0;">
                        ${b.sortBenchmark.speedupFactor.toFixed(1)}x Faster
                    </p>
                    <table class="viva-table">
                        <tr><td>Bubble Sort O(N²)</td><td>${b.sortBenchmark.bubbleSort.timeMilliseconds.toFixed(3)} ms</td></tr>
                        <tr><td>Introsort O(N log N)</td><td>${b.sortBenchmark.introsort.timeMilliseconds.toFixed(3)} ms</td></tr>
                    </table>
                </div>
            </div>
        `;
    }
}

async function runVivaCompareDS() {
    const area = document.getElementById('vivaBenchmarkOutput');
    area.innerHTML = `<p style="color:var(--accent);">Executing 50,000 push/pop operations on Hand-crafted Array DS vs STL...</p>`;

    const resp = await callApi('/compare-ds?iters=50000');
    if (resp.success && resp.data) {
        const c = resp.data;
        area.innerHTML = `
            <div style="display:grid; grid-template-columns:1fr 1fr; gap:16px;">
                <div class="viva-card">
                    <h4>Stack Comparison (50,000 operations)</h4>
                    <table class="viva-table" style="margin-top:10px;">
                        <tr><th>Type</th><th>Push</th><th>Pop</th></tr>
                        <tr><td><strong>ArrayStack</strong></td><td>${c.stackComparison.customArrayStackPushMicroseconds.toFixed(2)} µs</td><td>${c.stackComparison.customArrayStackPopMicroseconds.toFixed(2)} µs</td></tr>
                        <tr><td>std::stack</td><td>${c.stackComparison.stlStackPushMicroseconds.toFixed(2)} µs</td><td>${c.stackComparison.stlStackPopMicroseconds.toFixed(2)} µs</td></tr>
                    </table>
                </div>
                <div class="viva-card">
                    <h4>Queue Comparison (50,000 operations)</h4>
                    <table class="viva-table" style="margin-top:10px;">
                        <tr><th>Type</th><th>Enqueue</th><th>Dequeue</th></tr>
                        <tr><td><strong>CircularQueue</strong></td><td>${c.queueComparison.customCircularQueueEnqueueMicroseconds.toFixed(2)} µs</td><td>${c.queueComparison.customCircularQueueDequeueMicroseconds.toFixed(2)} µs</td></tr>
                        <tr><td>std::queue</td><td>${c.queueComparison.stlQueuePushMicroseconds.toFixed(2)} µs</td><td>${c.queueComparison.stlQueuePopMicroseconds.toFixed(2)} µs</td></tr>
                    </table>
                </div>
            </div>
        `;
    }
}

// ============================================================================
// INITIALIZATION
// ============================================================================

window.addEventListener('DOMContentLoaded', () => {
    loadStorefrontData();
    refreshCartView();

    const hash = window.location.hash.replace('#', '');
    if (hash === 'admin') navigateTo('admin');
    else if (hash === 'tracker') navigateTo('tracker');
});
