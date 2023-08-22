import tilt
from .query_graph import QueryGraphNode
from .operators import QuiltSource, TiltOpBuilder

class TiLTEnv :
    def __init__(self, num_threads = 1) :
        self.engine = tilt.engine()
        self.num_threads = num_threads
        self.compiled_op = None
        self.input_streams = []
        self.output_stream = None
        self.t_start = -1
        self.t_end = -1
        self.num_nodes = 0

    def add_node(self) :
        self.num_nodes +=1
        return self.num_nodes

    def set_start_time(self, t_start) :
        if self.compiled_op is not None:
            raise RuntimeError("Cannot modify start time after query compilation.")
        self.t_start = t_start

    def set_end_time(self, t_end) :
        if self.compiled_op is not None:
            raise RuntimeError("Cannot modify end time after query compilation.")
        self.t_end = t_end

    def create_input_from_dt(self, size : int, schema : tilt.DataType) :
        """ Add an input stream to the TiLTEnv
            Returns a QueryGraphNode that users can use to write QuiLT queries
        """
        if ((self.t_start < 0) or (self.t_end < 0) or (self.t_start >= self.t_end)) :
            raise RuntimeError("Execution start and end times must be specified before compiling.")
        in_stream_data = tilt.reg(size, schema)
        self.input_streams.append(in_stream_data)
        name = "in_stream_" + schema.str() + "_" + str(len(self.input_streams))
        return QueryGraphNode(self,
                              QuiltSource.from_datatype(name, schema, self.t_end - self.t_start),
                              data = in_stream_data)

    def compile_graph(self, graph, name = "query") :
        if ((self.t_start < 0) or (self.t_end < 0) or (self.t_start >= self.t_end)) :
            raise RuntimeError("Execution start and end times must be specified before compiling.")
        if self.compiled_op is not None :
            raise RuntimeError("Query graph has already been compiled.")

        # convert graph into TiLT IR
        op_builder = graph.build_tilt_op()
        tilt_op = op_builder.to_tilt_op()
        tilt.print_IR(tilt_op)

        # create executable from TiLT IR
        self.compiled_op = self.engine.compile(tilt_op, name)

        # set output within the environment and graph
        output_size = 0
        assert len(self.input_streams) > 0
        for input in self.input_streams :
            output_size = max(output_size, input.get_max_size())
        output_type = tilt_op.type.dtype
        output_stream = tilt.reg(output_size, output_type)
        self.output_stream = output_stream
        graph.data = output_stream

    def execute(self) :
        # check that query has been compiled and
        #   inputs + output have been set in environment
        if (self.compiled_op is None) or \
           (len(self.input_streams) == 0) or \
           (self.output_stream is None) :
            raise RuntimeError("Query must be compiled before execution.")

        self.engine.execute(self.compiled_op, self.t_start, self.t_end,
                            self.output_stream, *self.input_streams)
