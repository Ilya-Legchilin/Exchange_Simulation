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

void init_list(Order_TypeDef a)  // а- значение первого узла
{
  // выделение памяти под корень списка
  lst = (list*)malloc(sizeof(list));
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
    temp = (list*)malloc(sizeof(list));
    temp = lst;
    for (int i = 0; i < list_counter - 1; i++){
        printf("Iteration %d\n", i);
        if (temp->field.Price > temp->prev->field.Price)
            swap_orders(&(temp->field), &(temp->prev->field));
        else if (temp->field.Price == temp->prev->field.Price)
            if (temp->field.ID < temp->prev->field.ID)
                swap_orders(&(temp->field), &(temp->prev->field));
        if (i != list_counter - 1)
            temp = temp->prev;
    }
}


void sort_for_buy(void)
{
    list *temp;
    temp = (list*)malloc(sizeof(list));
    temp = lst;
    for (int i = 0; i < list_counter - 1; i++){
        printf("Iteration %d\n", i);
        if (temp->field.Price < temp->prev->field.Price)
            swap_orders(&(temp->field), &(temp->prev->field));
        else if (temp->field.Price == temp->prev->field.Price)
            if (temp->field.ID < temp->prev->field.ID)
                swap_orders(&(temp->field), &(temp->prev->field));
        if (i != list_counter - 1)
            temp = temp->prev;
    }
}


void addelem_to_list(Order_TypeDef order)
{
  list_counter++;
  list *temp, *p;
  temp = (list*)malloc(sizeof(list));
  p = lst->next;                            // сохранение указателя на следующий узел
  lst->next = temp;                         // предыдущий узел указывает на создаваемый
  temp->field = order;                      // сохранение поля данных добавляемого узла
  temp->next = p;                           // созданный узел указывает на следующий узел
  temp->prev = lst;                         // созданный узел указывает на предыдущий узел
  if (p != NULL)
    p->prev = temp;
  lst = temp;
  if ((state == WAIT_FOR_BUY) && (temp->field.side == S)){ // now a trade is about to happen
        state = WAIT_FOR_SELL;
        //trade(); //TODO
  } else if ((state == WAIT_FOR_SELL) && (temp->field.side == B)){
        state = WAIT_FOR_BUY;
        //trade(); //TODO
  } else {
        if (temp->field.side == B)
            sort_for_buy();
        else
            sort_for_sell();
  }
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


list * deletelem(void)
{
  list *prev, *next;
  prev = lst->prev; // узел, предшествующий lst
  next = lst->next; // узел, следующий за lst
  if (prev != NULL)
    prev->next = lst->next; // переставляем указатель
  if (next != NULL)
    next->prev = lst->prev; // переставляем указатель
  free(lst); // освобождаем память удаляемого элемента
  return(prev);
}


list * deletehead(void)
{
  list * root = lst;
  while (root->prev != NULL)
    root = root->prev;
  list *temp;
  temp = root->next;
  temp->prev = NULL;
  free(root);   // освобождение памяти текущего корня
  return(temp); // новый корень списка
}


void listprintr(void)
{
  list *p;
  p = lst;
  while (p->prev != NULL)
    p = p->prev;
  while (p != NULL){
    char c;
    if (p->field.side == B)
        c = 'B';
    else
        c = 'S';
    printf("O,%d,%c,Apples,%d,%.2f\n", p->field.ID, c, p->field.Qty, p->field.Price);
    p = p->next;
  } // условие окончания обхода
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
Order_TypeDef ** storage;
int storage_counter = 0;


void print_order(Order_TypeDef * order);
void trade(Order_TypeDef * buy_order, Order_TypeDef * sell_order);
void trade_show(int buy_id, int sell_id, int quantity, double price);
void cancel(int order_id);
void string_to_order(char * buffer, Order_TypeDef * order);


void trade(Order_TypeDef * buy_order, Order_TypeDef * sell_order){
    int minimum = min(buy_order->Qty, sell_order->Qty);
    if (buy_order->ID < sell_order->ID){
        printf("will be sold %d apples with the price %.2f\n", minimum, buy_order->Price);
        trade_show(buy_order->ID, sell_order->ID, minimum, buy_order->Price);
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
        trade_show(buy_order->ID, sell_order->ID, minimum, sell_order->Price);
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


void trade_show(int buy_id, int sell_id, int quantity, double price){
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
    // printf("from function id=%d side =%d quantity=%d price=%.2f\n", id, side, quantity, price);
}


int main()
{
    char line[32];
    scanf("%s", line);
    Order_TypeDef * order = (Order_TypeDef *)malloc(sizeof(Order_TypeDef));
    string_to_order(line, order);
    init_list(*order);
    int i = 0;
    while (1) {
        scanf("%s", line);
        Order_TypeDef * order = (Order_TypeDef *)malloc(sizeof(Order_TypeDef));
        string_to_order(line, order);
        char c;
        if (order->side == B)
            c = 'B';
        else
            c = 'S';
        // printf("O,%d,%c,Apples,%d,%.2f\n", order->ID, c, order->Qty, order->Price);
        addelem_to_list(*order);
        i++;
        printf("-----PRINT LIST-----\n");
        listprintr();
        printf("--------------------\n");
    }
    return 0;
}

/*
O,10,B,4,267.58
O,11,B,4,300.45
O,12,B,4,100.23
O,13,B,4,99.34
O,16,B,4,400.66
O,15,B,4,400.66
O,18,B,4,99.34
O,17,B,4,99.34
O,20,B,4,500.67
O,22,B,4,600.57
O,21,B,4,600.57
*/

