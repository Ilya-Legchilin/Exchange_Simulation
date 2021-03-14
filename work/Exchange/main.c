#include <stdio.h>
#include <stdlib.h>


typedef enum {
    B,
    S
} Side_TypeDef;


typedef enum {
    WAIT_FOR_SELL,
    WAIT_FOR_BUY,
    NONE
} State_TypeDef;


typedef struct {
    int ID;
    Side_TypeDef side;
    int Qty;
    double Price;
} Order_TypeDef;


typedef struct list
{
    Order_TypeDef field;  // поле данных
    struct list *next;    // указатель на следующий элемент
    struct list *prev;    // указатель на предыдущий элемент
} list;


list *lst;
int list_counter = 0;
State_TypeDef state = NONE;


void swap_orders(Order_TypeDef * a, Order_TypeDef * b);
void trade(Order_TypeDef * buy_order, Order_TypeDef * sell_order);
void deletetail(void);
void deletehead(void);
FILE * fpinput;
FILE * fpoutput;

void init_list(Order_TypeDef a)  // а- значение первого узла
{
    // выделение памяти под корень списка

    lst->field = a;
    lst->next = NULL; // указатель на следующий узел
    lst->prev = NULL; // указатель на предыдущий узел
    list_counter = 1;
    if (lst->field.side == B)
        state = WAIT_FOR_BUY;
    else
        state = WAIT_FOR_SELL;
}


void sort_for_sell(void)
{
    list *temp;
    temp = lst;
    for (int i = 0; i < list_counter - 1 && temp->prev != NULL; i++){
        if (temp->field.Price < temp->prev->field.Price)
            swap_orders(&(temp->field), &(temp->prev->field));
        else if (temp->field.Price == temp->prev->field.Price)
            if (temp->field.ID < temp->prev->field.ID)
                swap_orders(&(temp->field), &(temp->prev->field));
            temp = temp->prev;
    }
}


void sort_for_buy(void)
{
    list * temp = lst;
    for (int i = 0; i < list_counter - 1; i++) {
        if (temp->field.Price > temp->prev->field.Price)
            swap_orders(&(temp->field), &(temp->prev->field));
        else if (temp->field.Price == temp->prev->field.Price)
            if (temp->field.ID < temp->prev->field.ID)
                swap_orders(&(temp->field), &(temp->prev->field));
            temp = temp->prev;
    }
}


int addelem_to_list(Order_TypeDef order)
{
    if (lst != NULL) {
        list_counter++;
        list *temp, *p;
        temp = (list *) malloc(sizeof(list));
        p = (list *) malloc(sizeof(list));
        p = lst->next;                            // сохранение указателя на следующий узел
        lst->next = temp;                         // предыдущий узел указывает на создаваемый
        temp->field = order;                      // сохранение поля данных добавляемого узла
        temp->next = p;                           // созданный узел указывает на следующий узел
        temp->prev = lst;
        int i = 0;                                // созданный узел указывает на предыдущий узел
        if (p != NULL) {
            i++;
            p->prev = temp;
        }
        lst = temp;
        if ((state == WAIT_FOR_BUY) && (temp->field.side == S)) { // now a trade is about to happen
            while (list_counter > 1) {
                list *root = lst;
                while (root->prev != NULL)
                    root = root->prev;
                trade(&(root->field), &(temp->field));
                if (temp->field.Qty == 0) {
                    deletetail();
                    return 0;
                }
                if (root->field.Qty == 0)
                    deletehead();
            }
            if (list_counter == 1)
                state == WAIT_FOR_SELL;
        } else if ((state == WAIT_FOR_SELL) && (temp->field.side == B)) {
            while (list_counter > 1) {
                //printf("list counter === %d\n", list_counter);

                list * root = lst;
                while (root->prev != NULL)
                    root = root->prev;
                trade(&(root->field), &(temp->field));
                if (temp->field.Qty == 0) {
                    deletetail();
                    return 0;
                }
                if (root->field.Qty == 0)
                    deletehead();
            }
            if (list_counter == 1)
                state == WAIT_FOR_BUY;
        } else {
            if (temp->field.side == B)
                sort_for_buy();
            else
                sort_for_sell();
        }
    } else
        init_list(order);
    return 0;
}

void swap_orders(Order_TypeDef * a, Order_TypeDef * b){
    Order_TypeDef temp;
    temp.ID = a->ID;
    temp.Price = a->Price;
    temp.Qty = a->Qty;
    temp.side = a->side;
    a->ID = b->ID;
    a->Price = b->Price;
    a->Qty = b->Qty;
    a->side = b->side;

    b->ID = temp.ID;
    b->Price = temp.Price;
    b->Qty = temp.Qty;
    b->side = temp.side;
}


void deletetail(void)
{
    list_counter--;
    list * tail = lst;
    while (tail->next != NULL)
        tail = tail->next;
    lst = tail->prev;
    free(lst->next);
    lst->next = NULL;
}


void deletehead(void)
{
    list_counter--;
    list * root = lst;
    while (root->prev != NULL)
        root = root->prev;
    list *temp;
    temp = root->next;
    if (list_counter != 0){
        temp->prev = NULL;
        free(root);   // освобождение памяти текущего корня
    }
    while (temp->next != NULL)
        temp = temp->next;
    lst = temp;
}


int delete_via_id(int id)
{
    list * temp = lst;
    while (temp->field.ID != id && temp->prev != NULL)
        temp = temp->prev;
    if (temp->field.ID == id) {
        if (temp->next == NULL) {
            deletetail();
            cancel(id);
            return 0;
        }
        if (temp->prev == NULL) {
            deletehead();
            cancel(id);
            return 0;
        } else {
            list_counter--;
            temp->prev->next = temp->next;
            temp->next->prev = temp->prev;
            free(temp);
            cancel(id);
        }
    }
    return 0;
}


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


void trade(Order_TypeDef * buy_order, Order_TypeDef * sell_order);
void trade_show(int buy_id, int sell_id, int quantity, double price);
void cancel(int order_id);
int string_to_order(char * buffer, Order_TypeDef * order);


void trade(Order_TypeDef * buy_order, Order_TypeDef * sell_order){
    if (buy_order->side == S) {
        Order_TypeDef * temp = buy_order;
        buy_order = sell_order;
        sell_order = temp;
    }
    int minimum = min(buy_order->Qty, sell_order->Qty);
    if (buy_order->ID < sell_order->ID)
        trade_show(buy_order->ID, sell_order->ID, minimum, buy_order->Price);
    else
        trade_show(buy_order->ID, sell_order->ID, minimum, sell_order->Price);
    sell_order->Qty = sell_order->Qty - minimum;
    buy_order->Qty = buy_order->Qty - minimum;
}



void trade_show(int buy_id, int sell_id, int quantity, double price){
    trade_counter++;
    char k;
    if (buy_id < sell_id)
        k = 'B';
    else
        k = 'S';
    fprintf(fpoutput, "T,%d,%c,Apples,%d,%d,%d,%.2f\n", trade_counter, k, min(buy_id, sell_id), max(buy_id, sell_id), quantity, price);
    printf("T,%d,%c,Apples,%d,%d,%d,%.2f\n", trade_counter, k, min(buy_id, sell_id), max(buy_id, sell_id), quantity, price);
}


void cancel(int order_id){
    fprintf(fpoutput, "X,%d\n", order_id);
    printf("X,%d\n", order_id);
}


int string_to_order(char * buffer, Order_TypeDef * order){
    int i = 0;
    int id = 0;
    if (buffer[0] == 'C'){
        i = 2;
        while (buffer[i] != '\0')   {
            id = id*10 + buffer[i] - '0';
            i++;
        }
        return id;
    }
    Side_TypeDef side;
    int quantity = 0;
    char price_buffer[32];
    double price = 0;
    int counter_of_commas = 0;
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
        if (counter_of_commas == 3)
            quantity = quantity*10 + buffer[i] - '0';
        if (counter_of_commas == 4)
            price_buffer[j++] = buffer[i];
        i++;
    }
    price = atof(price_buffer);
    order->ID = id;
    order->Price = price;
    order->Qty = quantity;
    order->side = side;
    return 0;
}


void listprintr(void)
{
    list *p;
    p = lst;
    if (lst == NULL)
      printf("LIST IS EMPTY\n");
    else {
        while (p->prev != NULL) {
            p = p->prev;
        }
        while (p != NULL) {
            char c;
            if (p->field.side == B)
                c = 'B';
            else
                c = 'S';
            fprintf(fpoutput,"O,%d,%c,%d,%.2f\n", p->field.ID, c, p->field.Qty, p->field.Price);
            p = p->next;
        }
    }
}



int main()
{
    fpinput = fopen("input.txt", "r");
    fpoutput = fopen("output.txt", "w");
    lst = (list*)malloc(sizeof(list));
    char line[64];
    fscanf(fpinput, "%s", line);
    //scanf("%s", line);
    Order_TypeDef * order = (Order_TypeDef *)malloc(sizeof(Order_TypeDef));
    string_to_order(line, order);
    init_list(*order);
    int i = 0;
    int id = 0;
    while (!feof(fpinput)) {
        //printf("Here %d\n", i);
        fscanf(fpinput, "%s", line);
        //scanf("%s", line);
        id = string_to_order(line, order);
        if (id == 0){
            char c;
            if (order->side == B)
                c = 'B';
            else
                c = 'S';
            addelem_to_list(*order);
        }
        else
            delete_via_id(id);
        i++;
        fprintf(fpoutput, "-----PRINT LIST-----\n");
        listprintr();
        fprintf(fpoutput, "--------------------\n");
    }
    return 0;
}

/*
О,1,B,10,250.12
О,2,B,5,250.12
О,3,B,5,250.12
О,4,B,15,250.6
O,5,S,31,248.5
О,6,B,1,250.12
О,7,B,2,250.12
О,8,B,3,250.12
О,9,B,4,250.6
O,10,S,5,248.5
О,11,B,6,250.12
О,12,B,7,250.12
О,13,B,8,250.12
О,14,B,9,250.6
O,15,S,10,248.5
О,16,B,11,250.12
О,17,B,11,250.12
О,18,B,12,250.12
О,19,B,13,250.6
O,20,S,14,248.5
О,21,B,10,250.12
О,22,B,5,250.12
О,23,B,5,250.12
О,24,B,15,250.6
O,25,S,31,248.5
О,26,B,1,250.12
О,27,B,2,250.12
О,28,B,3,250.12
О,29,B,4,250.6
O,30,S,5,248.5
О,31,B,6,250.12
О,32,B,7,250.12
О,33,B,8,250.12
О,34,B,9,250.6
O,35,S,10,248.5
О,36,B,11,250.12
О,37,B,11,250.12
О,38,B,12,250.12
О,39,B,13,250.6
O,40,S,14,248.5
О,41,B,10,250.12
О,42,B,5,250.12
О,43,B,5,250.12
О,44,B,15,250.6
O,45,S,31,248.5
О,46,B,1,250.12
О,47,B,2,250.12
О,48,B,3,250.12
О,49,B,4,250.6
O,50,S,5,248.5
О,51,B,6,250.12
О,52,B,7,250.12
О,53,B,8,250.12
О,54,B,9,250.6
O,55,S,10,248.5
О,56,B,11,250.12
О,57,B,11,250.12
О,58,B,12,250.12
О,59,B,13,250.6
O,60,S,14,248.5
О,61,B,10,250.12
О,62,B,5,250.12
О,63,B,5,250.12
О,64,B,15,250.6
O,65,S,31,248.5
О,66,S,1000,250.12
О,67,S,2,250.12
О,68,S,4,250.6
O,70,S,5,248.5
О,71,S,6,250.12
О,72,S,7,250.12
О,73,S,8,250.12
О,74,S,9,250.6
O,75,S,10,248.5
О,76,S,11,250.12
О,77,S,12,250.12
О,79,B,13,250.6
O,80,B,14,248.5
О,81,B,10,250.12
О,82,B,5,250.12
О,83,B,5,250.12
О,84,B,15,250.6
O,85,S,31,248.5
О,86,B,1,250.12
О,87,B,2,250.12
О,88,B,3,250.12
О,89,B,4,250.6
O,90,S,5,248.5
О,91,B,6,250.12
О,92,B,7,250.12
О,93,B,8,250.12
О,94,B,9,250.6
O,95,S,10,248.5
О,96,B,11,250.12
О,97,B,11,250.12
О,98,B,12,250.12
О,99,B,13,250.6
O,100,S,14,248.5
О,121,B,10,250.12
О,122,B,5,250.12
О,123,B,5,250.12
О,124,B,15,250.6
O,125,S,31,248.5
О,126,B,1,250.12
О,127,B,2,250.12
О,128,B,3,250.12
О,129,B,4,250.6
O,130,S,5,248.5
О,131,B,6,250.12
О,132,B,7,250.12
О,133,B,8,250.12
О,134,B,9,250.6
O,135,S,10,248.5
О,136,B,11,250.12
О,137,B,11,250.12
О,138,B,12,250.12
О,139,B,13,250.6
O,140,S,14,248.5
О,141,B,10,250.12
О,142,B,5,250.12
О,143,B,5,250.12
О,144,B,15,250.6
O,145,S,31,248.5
О,146,B,1,250.12
О,147,B,2,250.12
О,148,B,3,250.12
О,149,B,4,250.6
O,150,S,5,248.5
О,151,B,6,250.12
О,152,B,7,250.12
О,153,B,8,250.12
О,154,B,9,250.6
O,155,S,10,248.5
О,156,B,11,250.12
О,157,B,11,250.12
О,158,B,12,250.12
О,159,B,13,250.6
O,160,S,14,248.5
О,161,B,10,250.12
О,162,B,5,250.12
О,163,B,5,250.12
О,164,B,15,250.6
O,165,S,31,248.5
О,166,S,1000,250.12
О,167,S,2,250.12
О,168,S,4,250.6
O,170,S,5,248.5
О,171,S,6,250.12
О,172,S,7,250.12
О,173,S,8,250.12
О,174,S,9,250.6
O,175,S,10,248.5
О,176,S,11,250.12
О,177,S,12,250.12
О,179,B,13,250.6
O,180,B,14,248.5
*/