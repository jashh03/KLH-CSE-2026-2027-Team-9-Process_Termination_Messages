#include <stdio.h>
#include <stdlib.h>

struct Node
{
    int data;
    struct Node *next;
};

void create_list(struct Node **head, int value)
{
    struct Node *newNode;

    newNode = (struct Node *)malloc(sizeof(struct Node));

    if (newNode == NULL)
    {
        printf("Memory allocation failed!\n");
        return;
    }

    newNode->data = value;
    newNode->next = *head;
    *head = newNode;
}

void display_list(struct Node *head)
{
    if (head == NULL)
    {
        printf("Linked list is empty.\n");
        return;
    }

    printf("Linked List: ");

    while (head != NULL)
    {
        printf("%d -> ", head->data);
        head = head->next;
    }

    printf("NULL\n");
}

void free_list(struct Node *head)
{
    struct Node *temp;

    while (head != NULL)
    {
        temp = head;
        head = head->next;
        free(temp);
    }
}

int main()
{
    int *array;
    int size = 2;
    int new_size;
    int n;
    struct Node *head = NULL;

    array = (int *)malloc(size * sizeof(int));

    if (array == NULL)
    {
        printf("Memory allocation failed!\n");
        return 1;
    }

    printf("\nInitial array size: %d\n", size);

    for (int i = 0; i < size; i++)
    {
        printf("Enter value %d: ", i + 1);
        scanf("%d", &array[i]);
    }

    printf("\nEnter new array size: ");
    scanf("%d", &new_size);

    int *temp = (int *)realloc(array, new_size * sizeof(int));

    if (temp == NULL)
    {
        printf("Memory resizing failed!\n");
        free(array);
        return 1;
    }

    array = temp;

    printf("\nArray resized successfully.\n");

        if (new_size > size)
    {
        for (int i = size; i < new_size; i++)
        {
            printf("Enter value %d: ", i + 1);
            scanf("%d", &array[i]);
        }
    }

    printf("\nArray elements:\n");

    for (int i = 0; i < new_size; i++)
        printf("%d ", array[i]);

    printf("\n");

    printf("\nHow many linked-list nodes? ");
    scanf("%d", &n);

    for (int i = 0; i < n; i++)
    {
        int value;

        printf("Enter value for node %d: ", i + 1);
        scanf("%d", &value);

        create_list(&head, value);
    }

    display_list(head);

    free(array);
    free_list(head);

    printf("\nAll dynamically allocated memory released.\n");
    printf("Program completed successfully.\n");

    return 0;
}
