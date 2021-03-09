#include <stdio.h>
#include <stdlib.h>



typedef enum {
    B,
    S
} Side_TypeDef;

typedef enum {
    WAIT_FOR_SELL,
    WAIT_FOR_BUY
} State_TypeDef;


typedef struct {
    int ID;
    Side_TypeDef side;
    int Qty;
    double Price;
} Order_TypeDef;


int min(int a, int b){
    if (a < b)
        return a;
    else
        return b;
}


int max(int a, int b){
    if  (a > b)
        return a;
    else
        return b;
}


int trade_counter = 0;
Order_TypeDef ** storage;
int storage_counter = 0;

void print_order(Order_TypeDef * order);

void compare(Order_TypeDef * buy_order, Order_TypeDef * sell_order){
    int minimum = min(buy_order->Qty, sell_order->Qty);
    if (buy_order->ID < sell_order->ID){
        printf("will be sold %d apples with the price %.2f\n", minimum, buy_order->Price);
        trade(buy_order->ID, sell_order->ID, minimum, buy_order->Price);
        if (minimum == buy_order->Qty){
            cancel(buy_order->ID);
            buy_order = NULL;
            sell_order->Qty = sell_order->Qty - minimum;
        } else{
            cancel(sell_order->ID);
            sell_order = NULL;
            buy_order->Qty = buy_order->Qty - minimum;
        }
    }
    else {
        printf("will be sold %d apples with the price %.2f\n", min(buy_order->Qty, sell_order->Qty), sell_order->Price);
        trade(buy_order->ID, sell_order->ID, minimum, sell_order->Price);
        if (minimum == buy_order->Qty){
            cancel(buy_order->ID);
            buy_order = NULL;
            sell_order->Qty = sell_order->Qty - minimum;
        } else{
            cancel(sell_order->ID);
            sell_order = NULL;
            buy_order->Qty = buy_order->Qty - minimum;
        }
    }
}


void trade(int buy_id, int sell_id, int quantity, double price){
    trade_counter++;
    printf("T,%d,B,Apples,%d,%d,%.2f\n", trade_counter, min(buy_id, sell_id), max(buy_id, sell_id), price);
}


void cancel(int order_id){
    printf("X,%d\n", order_id);
}


void string_to_order(char * buffer, Order_TypeDef * order){
    int i = 0;
    int id = 0;
    Side_TypeDef side;
    int quantity = 0;
    int counter_of_commas = 0;
    char price_buffer[32];
    double price = 0;
    int j = 0;
    while (buffer[i] != '\0'){
        if (buffer[i] == ','){
            counter_of_commas++;
            i++;
            continue;
        }
        if (counter_of_commas == 1)
            id = id*10 + buffer[i] - '0';
        if (counter_of_commas == 2)
            if (buffer[i] == 'B')
                side = B;
            else
                side = S;
        if (counter_of_commas == 4)
             quantity = quantity*10 + buffer[i] - '0';
        if (counter_of_commas == 5)
            price_buffer[j++] = buffer[i];
        i++;
    }
    price = atof(price_buffer);
    order->ID = id;
    order->Price = price;
    order->Qty = quantity;
    order->side = side;
    //printf("id=%d side =%d quantity=%d price=%.2f\n", id, side, quantity, price);
}


void push(Order_TypeDef * order){
    storage_counter++;
    storage = (Order_TypeDef **)realloc(storage, storage_counter*sizeof(Order_TypeDef));
    storage[storage_counter - 1] = order;
}


void print_storage(){
    printf("STORAGE--------\n");
    for (int i = 0; i < storage_counter; i++)
        print_order(storage[i]);
    printf("STORAGE END----\n");
}


void print_order(Order_TypeDef * order){
    char c;
    if (order->side == B)
        c = 'B';
    else
        c = 'S';
    printf("O,%d,%c,Apples,%d,%.2f\n", order->ID, c, order->Qty, order->Price);
}


int main()
{
    char line[32];
    while (1) {
        scanf("%s", line);
        Order_TypeDef * order = (Order_TypeDef *)malloc(sizeof(Order_TypeDef));
        string_to_order(line, order);
        push(order);
    }
    return 0;
}
