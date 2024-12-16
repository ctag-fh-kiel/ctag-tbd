import { useRef } from "preact/hooks";

interface QueueItem {
  url: string | URL;
  options?: RequestInit;
}

export default function useFetchQueue() {
  const queue = useRef<QueueItem[]>([]);
  const activeItem = useRef<QueueItem>();

  const runQueue = async () => {
    while (queue.current.length > 0) {
      const item = queue.current.shift();
      activeItem.current = item;

      if (item === undefined) {
        break;
      }

      await fetch(item.url, item.options);
      activeItem.current = undefined;
    }
  };

  const addToQueue = (item: QueueItem) => {
    queue.current.push(item);

    if (queue.current.length === 1 && activeItem.current === undefined) {
      runQueue();
    }
  };

  return { addToQueue };
}
