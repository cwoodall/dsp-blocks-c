#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>


typedef struct port
{
    float val;
    bool is_new;
} port_t;

void update_port(port_t *port, float val)
{
    printf("update port = %x\n", port);

    port->val = val;
    port->is_new = true;
}

bool read_port(port_t *port, float *val)
{
    printf("read port = %x\n", port);

    if (port->is_new)
    {
        port->is_new = false;
        *val = port->val;
        return true;
    }
    else
    {
        return false;
    }
}

typedef struct block
{
    int id;
    const char *name;
    port_t *in_ports;
    size_t num_in_ports;
    port_t *out_ports;
    size_t num_out_ports;

    int (*process)(struct block *ctx);

    void *data;
} block_t;

port_t ports[10] = {};

int add_fn(block_t *ctx)
{
    float a, b;

    if (read_port(&ctx->in_ports[0], &a) && read_port(&ctx->in_ports[1], &b))
    {
        update_port(&ctx->out_ports[0], a + b);
        return 0;
    }

    return 1;
}

int scale_fn(block_t *ctx)
{
    float a;
    const float scale_factor = 3.14;
    if (read_port(&ctx->in_ports[0], &a))
    {
        update_port(&ctx->out_ports[0], a * scale_factor);
        return 0;
    }
    return 1;
}

typedef struct filter {
    float prev_val;
} filter_t;

filter_t filter_ctx = {0};

int filter_fn(block_t *ctx)
{
    float a;
    filter_t *f =  (filter_t *)ctx->data;
    if (read_port(&ctx->in_ports[0], &a))
    {
        float next = (a + f->prev_val) /2;
        f->prev_val = next;
        update_port(&ctx->out_ports[0], next);
        return 0;
    }
    return 1;
}

block_t chain[] = {
    {
        .id = 0,
        .name = "add",
        .in_ports = &ports[0],
        .num_in_ports = 2,
        .out_ports = &ports[2],
        .num_out_ports = 1,
        .process = add_fn,
    },
    {
        .id = 1,
        .name = "scale",
        .in_ports = &ports[2],
        .num_in_ports = 1,
        .out_ports = &ports[3],
        .num_out_ports = 1,
        .process = scale_fn,

    },
    {
        .id = 1,
        .name = "filter",
        .in_ports = &ports[3],
        .num_in_ports = 1,
        .out_ports = &ports[4],
        .num_out_ports = 1,
        .process = filter_fn,
        .data = (void *)&filter_ctx
    },

};

int process_block(block_t *block)
{
    return block->process(block);
}

void print_block_inputs(block_t *block)
{
    printf("Block %s inputs = [", block->name);
    for (int i = 0; i < block->num_in_ports; i++)
    {
        printf("(%f, %s)", block->in_ports[i].val, block->in_ports[i].is_new ? "NEW" : "STALE");
        if (i != block->num_in_ports - 1)
        {
            printf(", ");
        }
    }
    printf("]\n");
}

void print_block_outputs(block_t *block)
{
    printf("Block %s outputs = [", block->name);
    for (int i = 0; i < block->num_out_ports; i++)
    {
        printf("(%f, %s)", block->out_ports[i].val, block->out_ports[i].is_new ? "NEW" : "STALE");
        if (i != block->num_out_ports - 1)
        {
            printf(", ");
        }
    }
    printf("]\n");
}

int process_chain(block_t *blocks, size_t n)
{

    printf("Num in chain = %i\n", n);

    for (int i = 0; i < n; i++)
    {
        print_block_inputs(&blocks[i]);
        if (0 != process_block(&blocks[i]))
        {
            return 1;
        }
        print_block_outputs(&blocks[i]);
    }
    return 1;
}

#define COUNTOF(c) (sizeof((c))/sizeof((c)[0]))
int main(int argc, const char *argv[])
{
    port_t *in0 = &ports[0];
    port_t *in1 = &ports[1];
    port_t *out = &ports[2];

    for (int i = 0; i < 100; i++)
    {
        update_port(in0, i);
        update_port(in1, 10);

        process_chain(chain, COUNTOF(chain));
    }

    return 0;
}