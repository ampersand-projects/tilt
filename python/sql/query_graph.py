from .operators import *

class QueryGraphNode :
    """
    Class representing a node in an operator graph
    Fields:
        op : QuiltOp for the operator corresponding to this node
        next : List of QueryGraphNodes for the next operators in the graph
        prev : List of QueryGraphNodes for the previous operators in the graph (up to 2)
        data : tilt.reg object corresponding to this node
            * data should only be used for Source and Output nodes
    """
    def __init__(self, env, op, *, next = [], prev = [], data = None) :
        self.env = env
        self.op = op
        self.next = next
        self.prev = prev
        self.data = data

    ### Operator Methods ###
    def map(self, map_fn) :
        if self.op.is_window() :
            raise RuntimeError("Map operator cannot be defined after a Window operator.")
        map_name = "map_" + str(self.env.add_node())
        map_node = QuiltMap(map_name, map_fn)
        self.next.append(QueryGraphNode(self.env, map_node, prev = [self]))
        return self.next[-1]

    def where(self, where_fn) :
        if self.op.is_window() :
            raise RuntimeError("Where operator cannot be defined after a Window operator.")
        where_name = "where_" + str(self.env.add_node())
        where_node = QuiltWhere(where_name, where_fn)
        self.next.append(QueryGraphNode(self.env, where_node, prev = [self]))
        return self.next[-1]

    def window(self, size, stride) :
        if self.op.is_window() :
            raise RuntimeError("Window operator cannot be defined after a Window operator.")
        window_name = "window_" + str(self.env.add_node())
        window_node = QuiltWindow(window_name, size, stride)
        self.next.append(QueryGraphNode(self.env, window_node, prev = [self]))
        return self.next[-1]

    def reduce(self, init, acc_fn) :
        if not self.op.is_window() :
            raise RuntimeError("Reduce operator must be defined immediately after a Window operator.")
        reduce_name = "reduce_" + str(self.env.add_node())
        reduce_node = QuiltReduce(reduce_name, init, acc_fn,
                                  self.op.get_stride(), self.op.get_size())
        self.next.append(QueryGraphNode(self.env, reduce_node, prev = [self]))
        return self.next[-1]

    def tjoin(self, right, join_fn) :
        if self.op.is_window() or right.op.is_window():
            raise RuntimeError("Map operator cannot be defined after a Window operator.")
        join_name = "join_" + str(self.env.add_node())
        join_node = QuiltTJoin(join_name, join_fn)
        self.next.append(QueryGraphNode(self.env, join_node, prev = [self, right]))
        return self.next[-1]

    ### Data-Writing Methods ###
    def commit_data(self, t) :
        if (self.data is None) :
            raise RuntimeError("Cannot commit data to a non-source node.")
        self.data.commit_data(t)

    def write_data(self, payload, t, i) :
        if (self.data is None) :
            raise RuntimeError("Cannot write data to a non-source node.")
        self.data.write_data(payload, t, i)

    def get_data_end_idx(self) :
        if (self.data is None) :
            raise RuntimeError("Cannot write data to a non-source node.")
        return self.data.get_end_idx()

    ### Data-Printing Methods ###
    def print_data(self) :
        if (self.data is None) :
            print("This operator has no data to print.")
        else :
            print(self.data)

    ### Compilation-Related Methods ###
    def build_tilt_op(self) :
        """ Return a TiLTOpBuilder corresponding to this node
        """
        # cannot compile graph where the last node is a window
        if ((self.op.is_window()) and (len(self.next) == 0)) :
            raise RuntimeError("Cannot compile graph with window as final operator.")

        # convert the previous nodes of the graph into TiLT IR
        op_builders = []
        if (len(self.prev) != 0) :
            for prev_op in self.prev :
                op_builders.append(prev_op.build_tilt_op())

        # update the op_builder to reflect this node's operator
        op_builder = self.op.modify_op_builders(op_builders)

        return op_builder
