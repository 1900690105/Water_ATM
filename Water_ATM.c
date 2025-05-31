/*
 * WATER ATM MANAGEMENT SYSTEM
 * 
 * This system manages a water dispensing ATM with smart payment optimization.
 * Key features:
 * - User registration and wallet management
 * - Multiple payment methods (Cash/Digital)
 * - Smart fee optimization strategies
 * - Discount system (student, bulk, loyalty)
 * - Pass system to avoid digital fees
 * - Analytics and reporting
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

// =================== SYSTEM CONSTANTS ===================
#define MAX_USERS 1000              // Maximum number of users in system
#define MAX_TRANSACTIONS 5000       // Maximum transaction history
#define WATER_PRICE_PER_LITER 2.0   // Base price per liter of water
#define DIGITAL_FEE 1.0             // Fee charged for digital payments
#define MIN_BULK_LITERS 10          // Minimum liters for bulk discount
#define LOYALTY_THRESHOLD 50.0      // Minimum spent to qualify for loyalty discount
#define WEEKLY_PASS_COST 15.0       // Cost of weekly pass (no digital fees)
#define MONTHLY_PASS_COST 50.0      // Cost of monthly pass (no digital fees)

// =================== DATA STRUCTURES ===================

/**
 * User Structure - Stores all user information
 * Contains personal info, financial data, and pass status
 */
typedef struct {
    int user_id;                    // Unique identifier for user
    char name[50];                  // User's full name
    char phone[15];                 // Contact number
    double wallet_balance;          // Current digital wallet balance
    double total_spent;             // Lifetime spending (for loyalty calculation)
    int transaction_count;          // Number of transactions made
    int loyalty_points;             // Points earned (1 point = ₹1 spent)
    int has_weekly_pass;            // Boolean: has active weekly pass
    int has_monthly_pass;           // Boolean: has active monthly pass
    time_t pass_expiry;             // When current pass expires
    int is_student;                 // Boolean: eligible for student discount
} User;

/**
 * Transaction Structure - Records each purchase
 * Maintains complete transaction history for analytics
 */
typedef struct {
    int transaction_id;             // Unique transaction identifier
    int user_id;                    // Which user made this transaction
    double amount;                  // Final amount paid
    double liters;                  // Quantity of water purchased
    char payment_method[20];        // "Cash" or "Digital"
    double fee_charged;             // Digital payment fee (if any)
    double discount_applied;        // Total discount given
    time_t timestamp;               // When transaction occurred
} Transaction;

/**
 * Analytics Structure - System-wide statistics
 * Tracks business metrics and performance indicators
 */
typedef struct {
    double total_revenue;           // Total water sales revenue
    double total_fees_collected;    // Total digital payment fees collected
    double total_discounts_given;   // Total discounts provided
    int cash_transactions;          // Count of cash payments
    int digital_transactions;       // Count of digital payments
    int bulk_purchases;             // Count of bulk orders (≥10L)
    int pass_holders;               // Count of users with active passes
} Analytics;

// =================== GLOBAL VARIABLES ===================
User users[MAX_USERS];              // Array to store all registered users
Transaction transactions[MAX_TRANSACTIONS]; // Transaction history
Analytics stats = {0};             // System statistics (initialized to zero)
int user_count = 0;                 // Current number of registered users
int transaction_count = 0;          // Current number of transactions

// =================== FUNCTION DECLARATIONS ===================
void display_menu();               // Show main menu options
void register_user();              // Register new user in system
void top_up_wallet();              // Add money to user's digital wallet
void purchase_water();             // Main water purchase flow
void purchase_pass();              // Buy weekly/monthly pass
void view_user_profile();          // Display user information
void admin_analytics();            // Show system analytics
double calculate_discount(User* user, double liters, char* payment_method);
double calculate_bulk_discount(double liters);
double calculate_loyalty_discount(User* user);
int is_pass_valid(User* user);     // Check if user's pass is still active
void update_loyalty_points(User* user, double amount);
void save_transaction(int user_id, double amount, double liters, char* method, double fee, double discount);
User* find_user(int user_id);      // Find user by ID
void display_pricing_info();       // Show pricing and discount information

// =================== MAIN PROGRAM FLOW ===================
/**
 * Main function - Entry point of the program
 * Displays welcome message and runs main menu loop
 */
int main() {
    int choice;
    
    // Display system welcome message
    printf("=== WATER ATM MANAGEMENT SYSTEM ===\n");
    printf("Smart Solution for Digital Payment Optimization\n\n");
    
    // Main program loop - continues until user chooses to exit
    while (1) {
        display_menu();             // Show available options
        printf("Enter your choice: ");
        scanf("%d", &choice);
        
        // Process user's menu choice
        switch (choice) {
            case 1:
                register_user();    // Add new user to system
                break;
            case 2:
                top_up_wallet();    // Add money to wallet
                break;
            case 3:
                purchase_water();   // Main purchase functionality
                break;
            case 4:
                purchase_pass();    // Buy fee-waiver pass
                break;
            case 5:
                view_user_profile(); // Show user details
                break;
            case 6:
                display_pricing_info(); // Show pricing guide
                break;
            case 7:
                admin_analytics();  // Show business analytics
                break;
            case 8:
                printf("Thank you for using Water ATM System!\n");
                exit(0);            // Clean program exit
            default:
                printf("Invalid choice! Please try again.\n");
        }
        
        // Pause for user to read output before clearing screen
        printf("\nPress Enter to continue...");
        getchar();
        getchar();
        system("clear || cls");     // Clear screen (works on Linux/Windows)
    }
    
    return 0;
}

// =================== USER INTERFACE FUNCTIONS ===================

/**
 * Display Main Menu
 * Shows all available system functions to user
 */
void display_menu() {
    printf("\n=== MAIN MENU ===\n");
    printf("1. Register New User\n");
    printf("2. Top-up Wallet\n");
    printf("3. Purchase Water\n");
    printf("4. Purchase Weekly/Monthly Pass\n");
    printf("5. View User Profile\n");
    printf("6. View Pricing & Discount Info\n");
    printf("7. Admin Analytics\n");
    printf("8. Exit\n");
    printf("==================\n");
}

// =================== USER MANAGEMENT FUNCTIONS ===================

/**
 * Register New User
 * Collects user information and creates account
 * Initializes all user fields with default values
 */
void register_user() {
    // Check if system has reached maximum user capacity
    if (user_count >= MAX_USERS) {
        printf("Maximum user limit reached!\n");
        return;
    }
    
    // Get pointer to next available user slot
    User* new_user = &users[user_count];
    new_user->user_id = user_count + 1;    // Assign unique ID
    
    printf("\n=== USER REGISTRATION ===\n");
    
    // Collect user information
    printf("Enter name: ");
    scanf(" %[^\n]", new_user->name);      // Read full name including spaces
    
    printf("Enter phone number: ");
    scanf("%s", new_user->phone);
    
    printf("Are you a student? (1 for Yes, 0 for No): ");
    scanf("%d", &new_user->is_student);
    
    // Initialize financial and usage data
    new_user->wallet_balance = 0.0;        // Start with empty wallet
    new_user->total_spent = 0.0;           // No purchase history
    new_user->transaction_count = 0;       // No transactions yet
    new_user->loyalty_points = 0;          // No loyalty points
    new_user->has_weekly_pass = 0;         // No active passes
    new_user->has_monthly_pass = 0;
    new_user->pass_expiry = 0;             // No expiry date
    
    user_count++;                          // Increment total user count
    
    // Confirm successful registration
    printf("\nRegistration successful!\n");
    printf("Your User ID: %d\n", new_user->user_id);
    if (new_user->is_student) {
        printf("Student discount: 10%% off on all purchases!\n");
    }
}

/**
 * Top-up Wallet
 * Allows users to add money to their digital wallet
 * Includes bonus system for large top-ups
 */
void top_up_wallet() {
    int user_id;
    double amount;
    
    printf("\n=== WALLET TOP-UP ===\n");
    printf("Enter User ID: ");
    scanf("%d", &user_id);
    
    // Find the user in system
    User* user = find_user(user_id);
    if (!user) {
        printf("User not found!\n");
        return;
    }
    
    // Display current balance and get top-up amount
    printf("Current wallet balance: ₹%.2f\n", user->wallet_balance);
    printf("Enter amount to add: ₹");
    scanf("%lf", &amount);
    
    // Validate amount
    if (amount <= 0) {
        printf("Invalid amount!\n");
        return;
    }
    
    // Add money to wallet
    user->wallet_balance += amount;
    printf("Wallet topped up successfully!\n");
    printf("New balance: ₹%.2f\n", user->wallet_balance);
    
    // Bonus system: Give 2% bonus for top-ups ≥ ₹100
    if (amount >= 100) {
        double bonus = amount * 0.02;      // Calculate 2% bonus
        user->wallet_balance += bonus;     // Add bonus to wallet
        printf("Bonus added: ₹%.2f (2%% bonus for top-up ≥ ₹100)\n", bonus);
        printf("Final balance: ₹%.2f\n", user->wallet_balance);
    }
}

// =================== CORE BUSINESS LOGIC ===================

/**
 * Purchase Water - Main Business Function
 * Handles the complete water purchase process including:
 * - User validation
 * - Payment method selection
 * - Discount calculation
 * - Fee optimization
 * - Transaction recording
 */
void purchase_water() {
    int user_id;
    double liters;
    int payment_choice;
    
    printf("\n=== WATER PURCHASE ===\n");
    printf("Enter User ID: ");
    scanf("%d", &user_id);
    
    // Validate user exists
    User* user = find_user(user_id);
    if (!user) {
        printf("User not found!\n");
        return;
    }
    
    // Get quantity needed
    printf("Enter liters of water needed: ");
    scanf("%lf", &liters);
    
    if (liters <= 0) {
        printf("Invalid quantity!\n");
        return;
    }
    
    // Calculate base cost (before fees/discounts)
    double base_cost = liters * WATER_PRICE_PER_LITER;
    
    // Payment method selection
    printf("\n=== PAYMENT OPTIONS ===\n");
    printf("1. Cash (No extra fee)\n");
    printf("2. Digital Payment (Wallet/UPI)\n");
    printf("Choose payment method: ");
    scanf("%d", &payment_choice);
    
    // Initialize transaction variables
    char payment_method[20];
    double fee = 0.0;              // Digital payment fee
    double discount = 0.0;         // Total discount applied
    double final_amount = base_cost;
    
    // Process payment method choice
    if (payment_choice == 1) {
        // ===== CASH PAYMENT PROCESSING =====
        strcpy(payment_method, "Cash");
        discount = calculate_discount(user, liters, payment_method);
        final_amount = base_cost - discount;
        stats.cash_transactions++;
        
    } else if (payment_choice == 2) {
        // ===== DIGITAL PAYMENT PROCESSING =====
        strcpy(payment_method, "Digital");
        
        // SMART FEE OPTIMIZATION LOGIC
        // Check if user has valid pass (no fee if pass active)
        if (is_pass_valid(user)) {
            printf("Pass holder - No digital payment fee!\n");
            fee = 0.0;
        } else {
            // Calculate available discounts
            discount = calculate_discount(user, liters, payment_method);
            
            // Fee optimization strategies:
            if (liters >= MIN_BULK_LITERS) {
                // Strategy 1: Bulk purchase - waive fee
                printf("Bulk purchase - Digital fee waived!\n");
                fee = 0.0;
            } else if (discount >= DIGITAL_FEE) {
                // Strategy 2: Discount covers fee
                printf("Discount covers digital fee!\n");
                fee = 0.0;
            } else {
                // Strategy 3: Reduce fee by available discount
                fee = DIGITAL_FEE - discount;
                if (fee < 0) fee = 0;
            }
        }
        
        final_amount = base_cost - discount + fee;
        
        // Validate sufficient wallet balance
        if (user->wallet_balance < final_amount) {
            printf("Insufficient wallet balance!\n");
            printf("Required: ₹%.2f, Available: ₹%.2f\n", final_amount, user->wallet_balance);
            return;
        }
        
        // Deduct amount from wallet
        user->wallet_balance -= final_amount;
        stats.digital_transactions++;
        
    } else {
        printf("Invalid payment method!\n");
        return;
    }
    
    // ===== UPDATE USER STATISTICS =====
    user->total_spent += base_cost;        // Track lifetime spending
    user->transaction_count++;             // Increment transaction count
    update_loyalty_points(user, base_cost); // Award loyalty points
    
    // Track bulk purchases for analytics
    if (liters >= MIN_BULK_LITERS) {
        stats.bulk_purchases++;
    }
    
    // ===== RECORD TRANSACTION =====
    save_transaction(user_id, final_amount, liters, payment_method, fee, discount);
    
    // ===== UPDATE GLOBAL STATISTICS =====
    stats.total_revenue += base_cost;
    stats.total_fees_collected += fee;
    stats.total_discounts_given += discount;
    
    // ===== DISPLAY PURCHASE RECEIPT =====
    printf("\n=== PURCHASE RECEIPT ===\n");
    printf("User: %s (ID: %d)\n", user->name, user->user_id);
    printf("Water quantity: %.2f liters\n", liters);
    printf("Base cost: ₹%.2f\n", base_cost);
    if (discount > 0) {
        printf("Discount applied: -₹%.2f\n", discount);
    }
    if (fee > 0) {
        printf("Digital payment fee: +₹%.2f\n", fee);
    }
    printf("Final amount: ₹%.2f\n", final_amount);
    printf("Payment method: %s\n", payment_method);
    if (payment_choice == 2) {
        printf("Remaining wallet balance: ₹%.2f\n", user->wallet_balance);
    }
    printf("Loyalty points earned: +%d\n", (int)(base_cost));
    printf("Total loyalty points: %d\n", user->loyalty_points);
    printf("========================\n");
}

/**
 * Purchase Pass System
 * Allows users to buy weekly/monthly passes to avoid digital fees
 * This is a key strategy for frequent digital payment users
 */
void purchase_pass() {
    int user_id, pass_type;
    
    printf("\n=== PURCHASE PASS ===\n");
    printf("Enter User ID: ");
    scanf("%d", &user_id);
    
    User* user = find_user(user_id);
    if (!user) {
        printf("User not found!\n");
        return;
    }
    
    // Display pass options
    printf("\n=== PASS OPTIONS ===\n");
    printf("1. Weekly Pass - ₹%.2f (No digital fees for 7 days)\n", WEEKLY_PASS_COST);
    printf("2. Monthly Pass - ₹%.2f (No digital fees for 30 days)\n", MONTHLY_PASS_COST);
    printf("Choose pass type: ");
    scanf("%d", &pass_type);
    
    double pass_cost;
    int pass_days;
    
    // Set pass parameters based on selection
    if (pass_type == 1) {
        pass_cost = WEEKLY_PASS_COST;
        pass_days = 7;
    } else if (pass_type == 2) {
        pass_cost = MONTHLY_PASS_COST;
        pass_days = 30;
    } else {
        printf("Invalid pass type!\n");
        return;
    }
    
    // Check wallet balance
    if (user->wallet_balance < pass_cost) {
        printf("Insufficient wallet balance!\n");
        printf("Required: ₹%.2f, Available: ₹%.2f\n", pass_cost, user->wallet_balance);
        return;
    }
    
    // Process pass purchase
    user->wallet_balance -= pass_cost;
    
    // Activate appropriate pass
    if (pass_type == 1) {
        user->has_weekly_pass = 1;
    } else {
        user->has_monthly_pass = 1;
    }
    
    // Set expiry time (current time + pass duration)
    user->pass_expiry = time(NULL) + (pass_days * 24 * 60 * 60);
    stats.pass_holders++;
    
    // Confirm purchase
    printf("Pass purchased successfully!\n");
    printf("Cost: ₹%.2f\n", pass_cost);
    printf("Valid for: %d days\n", pass_days);
    printf("Remaining wallet balance: ₹%.2f\n", user->wallet_balance);
    printf("Benefit: No digital payment fees during pass validity!\n");
}

// =================== INFORMATION DISPLAY FUNCTIONS ===================

/**
 * View User Profile
 * Displays comprehensive user information and provides usage insights
 */
void view_user_profile() {
    int user_id;
    
    printf("\n=== USER PROFILE ===\n");
    printf("Enter User ID: ");
    scanf("%d", &user_id);
    
    User* user = find_user(user_id);
    if (!user) {
        printf("User not found!\n");
        return;
    }
    
    // Display basic user information
    printf("\n=== PROFILE DETAILS ===\n");
    printf("Name: %s\n", user->name);
    printf("User ID: %d\n", user->user_id);
    printf("Phone: %s\n", user->phone);
    printf("Student: %s\n", user->is_student ? "Yes" : "No");
    printf("Wallet Balance: ₹%.2f\n", user->wallet_balance);
    printf("Total Spent: ₹%.2f\n", user->total_spent);
    printf("Transactions: %d\n", user->transaction_count);
    printf("Loyalty Points: %d\n", user->loyalty_points);
    
    // Display pass status
    if (is_pass_valid(user)) {
        time_t now = time(NULL);
        int days_left = (user->pass_expiry - now) / (24 * 60 * 60);
        printf("Active Pass: %s (%d days remaining)\n", 
               user->has_monthly_pass ? "Monthly" : "Weekly", days_left);
    } else {
        printf("Active Pass: None\n");
    }
    
    // Provide cost optimization suggestion
    double potential_monthly_fees = user->transaction_count * DIGITAL_FEE;
    printf("\nPotential monthly digital fees: ₹%.2f\n", potential_monthly_fees);
    if (potential_monthly_fees > MONTHLY_PASS_COST) {
        printf("💡 Tip: Monthly pass could save you ₹%.2f!\n", 
               potential_monthly_fees - MONTHLY_PASS_COST);
    }
}

/**
 * Display Pricing Information
 * Shows complete pricing structure and cost optimization strategies
 */
void display_pricing_info() {
    printf("\n=== PRICING & DISCOUNTS ===\n");
    printf("Base Price: ₹%.2f per liter\n", WATER_PRICE_PER_LITER);
    printf("Digital Payment Fee: ₹%.2f (when applicable)\n", DIGITAL_FEE);
    
    // Show fee avoidance strategies
    printf("\n=== WAYS TO AVOID DIGITAL FEES ===\n");
    printf("1. Weekly Pass (₹%.2f) - No fees for 7 days\n", WEEKLY_PASS_COST);
    printf("2. Monthly Pass (₹%.2f) - No fees for 30 days\n", MONTHLY_PASS_COST);
    printf("3. Bulk Purchase - Buy ≥%d liters (fee waived)\n", MIN_BULK_LITERS);
    printf("4. Student Discount - 10%% off (may cover fee)\n");
    printf("5. Loyalty Discount - Spend ≥₹%.2f total (5%% off)\n", LOYALTY_THRESHOLD);
    
    printf("\n=== WALLET BONUSES ===\n");
    printf("• Top-up ≥₹100: Get 2%% bonus credit\n");
    
    printf("\n=== LOYALTY PROGRAM ===\n");
    printf("• Earn 1 point per ₹1 spent\n");
    printf("• 100 points = ₹5 discount on next purchase\n");
    
    // Show cost comparison example
    printf("\n=== COST COMPARISON EXAMPLE ===\n");
    printf("Daily 5L purchase for 30 days:\n");
    printf("• Cash: ₹%.2f\n", 30 * 5 * WATER_PRICE_PER_LITER);
    printf("• Digital (no pass): ₹%.2f\n", 30 * (5 * WATER_PRICE_PER_LITER + DIGITAL_FEE));
    printf("• Digital (monthly pass): ₹%.2f\n", MONTHLY_PASS_COST + 30 * 5 * WATER_PRICE_PER_LITER);
    printf("• Savings with pass: ₹%.2f\n", 30 * DIGITAL_FEE - MONTHLY_PASS_COST);
}

/**
 * Admin Analytics
 * Displays comprehensive system analytics and business insights
 */
void admin_analytics() {
    printf("\n=== ADMIN ANALYTICS ===\n");
    
    // User and transaction statistics
    printf("Total Users: %d\n", user_count);
    printf("Total Transactions: %d\n", transaction_count);
    printf("Cash Transactions: %d (%.1f%%)\n", stats.cash_transactions, 
           transaction_count > 0 ? (stats.cash_transactions * 100.0 / transaction_count) : 0);
    printf("Digital Transactions: %d (%.1f%%)\n", stats.digital_transactions,
           transaction_count > 0 ? (stats.digital_transactions * 100.0 / transaction_count) : 0);
    printf("Bulk Purchases: %d\n", stats.bulk_purchases);
    printf("Pass Holders: %d\n", stats.pass_holders);
    
    // Financial summary
    printf("\n=== FINANCIAL SUMMARY ===\n");
    printf("Total Revenue: ₹%.2f\n", stats.total_revenue);
    printf("Fees Collected: ₹%.2f\n", stats.total_fees_collected);
    printf("Discounts Given: ₹%.2f\n", stats.total_discounts_given);
    printf("Net Revenue: ₹%.2f\n", stats.total_revenue + stats.total_fees_collected - stats.total_discounts_given);
    
    // Business recommendations based on data
    printf("\n=== RECOMMENDATIONS ===\n");
    if (stats.digital_transactions < stats.cash_transactions) {
        printf("• Consider promoting passes to increase digital adoption\n");
        printf("• Bulk purchase incentives are working well\n");
    }
    if (stats.pass_holders < user_count * 0.3) {
        printf("• Low pass adoption - consider promotional pricing\n");
    }
}

// =================== CALCULATION FUNCTIONS ===================

/**
 * Calculate Total Discount
 * Combines all applicable discounts for a user's purchase
 * This is where the smart optimization happens
 */
double calculate_discount(User* user, double liters, char* payment_method) {
    double discount = 0.0;
    
    // Student discount: 10% off base cost
    if (user->is_student) {
        discount += (liters * WATER_PRICE_PER_LITER) * 0.10;
    }
    
    // Bulk purchase discount: Fixed amount based on quantity
    if (liters >= MIN_BULK_LITERS) {
        discount += calculate_bulk_discount(liters);
    }
    
    // Loyalty discount: Percentage of total lifetime spending
    if (user->total_spent >= LOYALTY_THRESHOLD) {
        discount += calculate_loyalty_discount(user);
    }
    
    // Loyalty points redemption: 100 points = ₹5
    if (user->loyalty_points >= 100) {
        discount += 5.0;
        user->loyalty_points -= 100;    // Deduct redeemed points
    }
    
    return discount;
}

/**
 * Calculate Bulk Purchase Discount
 * Tiered discount system based on quantity
 */
double calculate_bulk_discount(double liters) {
    if (liters >= 20) return 4.0;      // ₹4 discount for 20L+
    if (liters >= 15) return 3.0;      // ₹3 discount for 15L+
    if (liters >= 10) return 2.0;      // ₹2 discount for 10L+
    return 0.0;
}

/**
 * Calculate Loyalty Discount
 * Returns 5% of user's total lifetime spending as discount
 */
double calculate_loyalty_discount(User* user) {
    return (user->total_spent * 0.05);
}

// =================== UTILITY FUNCTIONS ===================

/**
 * Check Pass Validity
 * Determines if user's pass is currently active
 */
int is_pass_valid(User* user) {
    // Check if user has a pass and it hasn't expired
    if ((user->has_weekly_pass || user->has_monthly_pass) && 
        time(NULL) < user->pass_expiry) {
        return 1;   // Pass is valid
    }
    return 0;       // No valid pass
}

/**
 * Update Loyalty Points
 * Awards points based on amount spent (1 point per rupee)
 */
void update_loyalty_points(User* user, double amount) {
    user->loyalty_points += (int)amount;
}

/**
 * Save Transaction Record
 * Stores transaction details in system history
 */
void save_transaction(int user_id, double amount, double liters, char* method, double fee, double discount) {
    // Check if transaction storage is full
    if (transaction_count >= MAX_TRANSACTIONS) return;
    
    // Create new transaction record
    Transaction* txn = &transactions[transaction_count];
    txn->transaction_id = transaction_count + 1;
    txn->user_id = user_id;
    txn->amount = amount;
    txn->liters = liters;
    strcpy(txn->payment_method, method);
    txn->fee_charged = fee;
    txn->discount_applied = discount;
    txn->timestamp = time(NULL);        // Current timestamp
    
    transaction_count++;                // Increment transaction counter
}

/**
 * Find User by ID
 * Searches user array and returns pointer to user or NULL if not found
 */
User* find_user(int user_id) {
    // Linear search through user array
    for (int i = 0; i < user_count; i++) {
        if (users[i].user_id == user_id) {
            return &users[i];           // Return pointer to found user
        }
    }
    return NULL;                        // User not found
}
